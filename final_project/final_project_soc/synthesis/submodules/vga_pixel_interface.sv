module vga_text_avl_interface (
	// Avalon Clock Input, note this clock is also used for VGA, so this must be 50Mhz
	// We can put a clock divider here in the future to make this IP more generalizable
	input logic CLK,
	
	// Avalon Reset Input
	input logic RESET,
	
	// Avalon-MM Slave Signals
	input  logic AVL_READ,					// Avalon-MM Read
	input  logic AVL_WRITE,					// Avalon-MM Write
	input  logic AVL_CS,					// Avalon-MM Chip Select
	input  logic [3:0] AVL_BYTE_EN,			// Avalon-MM Byte Enable
	input  logic [14:0] AVL_ADDR,			// Avalon-MM Address
	input  logic [31:0] AVL_WRITEDATA,		// Avalon-MM Write Data
	output logic [31:0] AVL_READDATA,		// Avalon-MM Read Data
	
	// Exported Conduit (mapped to VGA port - make sure you export in Platform Designer)
	output logic [3:0]  red, green, blue,	// VGA color channels (mapped to output pins in top-level)
	output logic hs, vs						// VGA HS/VS
);

// Rendering macros
`define SIDEBAR_WIDTH    128
`define INVERSION_HEIGHT 20
`define LOGO_HEIGHT      32

// Text box
`define TEXTBOX_Y      454
`define TEXTBOX_Y_END  473
`define TEXTBOX_X      148
`define TEXTBOX_X_END  620

// Relevant VGA signals
logic pixel_clk, blank;
logic [9:0] x, y;

// Instantiate the VGA controller
vga_controller vga_control(
	// Inputs
	.Clk(CLK),					// 50Mhz Clock
	.Reset(RESET),				// Avalon Reset Input
	// Outputs
	.hs(hs),					// Horizontal sync
	.vs(vs),					// Vertical sync
	.pixel_clk(pixel_clk),		// Pulse for every pixel drawn
	.blank(blank),				// Indicates blank, non-drawable pixel
	.sync(),					// Not used
	.DrawX(x),					// X-coordinate
	.DrawY(y)					// Y-coordinate
);

// ------ START OF VRAM ------ 

// 0x0000-0x4AFF
// On-chip memory
ram ram(
	// Input
	.wraddress(AVL_ADDR),
	.rdaddress(vram_read_addr),
	.byteena_a(AVL_BYTE_EN),
	.wrclock(CLK),
	.rdclock(pixel_clk),
	.data(AVL_WRITEDATA),
	.wren(AVL_CS & AVL_WRITE),

	// Output
	.q(vram_data)
);

// 0x4B00-0x4B03
// Palette registers 
logic [3:0] [11:0] palette;

// 0x4B04-0x4B07
// Control registers
//      0x4B04: y-coordinate for start of selected text on sidebar
logic [3:0] [31:0] control;

// ------ END OF VRAM ------ 

// 4-bit index into the palette
logic [1:0] pixel_data;

// VRAM read wires
logic [14:0] vram_read_addr;
logic [31:0] vram_data;

// Look-ahead cache data and pixel positions
logic [31:0] vram_data_cache;
logic [9:0] x_adjusted;
logic [9:0] y_adjusted;

// Pixel color
logic [11:0] color;

// Registers for VGA sync
logic [3:0] red_next;
logic [3:0] green_next;
logic [3:0] blue_next;

// Lookahead by one values
logic [9:0] x_la;
logic [9:0] y_la;

// Conditional for color inversion
logic invert;

// VRAM interaction logic
always_comb begin
	// X and y adjusted are 4 bits ahead of x and y
	x_adjusted = (x + 4) % 800;
	y_adjusted = x_adjusted < 4 ? y+1 : y;

	// VRAM address for the pixel data of the pixel to draw
	vram_read_addr = (y_adjusted*40)+(x_adjusted[9:4]);

	// Index of the pixel to draw into the color palette
	pixel_data = vram_data_cache >> (30-(x[3:0]*2));

	// Get the foreground and background colors from the palette
	color = palette[pixel_data];

	// Lookahead by one values
	x_la = (x + 1) % 800;
	y_la = x_la < 1 ? y+1 : y;

    // Conditional for color inversion
    invert = y_la >= control[0] & y_la < control[0]+`INVERSION_HEIGHT & x_la < `SIDEBAR_WIDTH;

	// Set the next colors to display on-screen
	if (pixel_data != 0) begin
        if (invert) begin
            red_next   = 16-color[11:8]; // Invert colors
            green_next = 16-color[7:4];
            blue_next  = 16-color[3:0];
        end else if (y_la < `LOGO_HEIGHT & x_la < `SIDEBAR_WIDTH) begin
            red_next   = 4'h5; // Blurple
            green_next = 4'h6;
            blue_next  = 4'hF;
        end else begin
            red_next   = color[11:8];   // Keep colors the same
            green_next = color[7:4];
            blue_next  = color[3:0];
        end
	end else begin
		if (x_la < `SIDEBAR_WIDTH) begin
            if (y_la < `LOGO_HEIGHT) begin
                red_next   = 4'h3;  // Dark discord grey
                green_next = 4'h3;
                blue_next  = 4'h3;
            end else begin
                red_next   = invert ? 4'hF : 4'h3; // Dark discord grey, white if inverted
                green_next = invert ? 4'hF : 4'h3;
                blue_next  = invert ? 4'hF : 4'h3;
            end
		end else begin
			if (x_la < `TEXTBOX_X_END & x_la >= `TEXTBOX_X & y_la < `TEXTBOX_Y_END & y_la >= `TEXTBOX_Y) begin
				red_next   = 4'h3; // Dark discord grey
				green_next = 4'h3;
				blue_next  = 4'h3;
			end else begin
				red_next   = 4'h4; // Light discord grey
				green_next = 4'h4;
				blue_next  = 4'h4;
			end
		end
	end

	// Dummy read values
	AVL_READDATA = 32'h0000;
end

always_ff @ (posedge CLK) begin
	// Load the cached
	if(x % 8 == 0) begin
		vram_data_cache <= vram_data;
	end

	// Writing to the palette registers, no byteenable
	if(AVL_CS & AVL_WRITE & (AVL_ADDR > 16'h4AFF & AVL_ADDR < 16'h4B04)) begin
		palette[AVL_ADDR[1:0]] <= AVL_WRITEDATA[11:0];
	end

    // Writing to the control registers, no byteenable
	if(AVL_CS & AVL_WRITE & (AVL_ADDR >= 16'h4B04)) begin
		control[AVL_ADDR[1:0]] <= AVL_WRITEDATA[11:0];
	end
end

// Pixel drawing
always_ff @ (posedge pixel_clk) 
begin
	red <= red_next & {4{blank}};
	green <= green_next & {4{blank}};
	blue <= blue_next & {4{blank}};
end

endmodule
