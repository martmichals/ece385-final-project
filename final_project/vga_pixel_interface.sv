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
`define SIDEBAR_WIDTH  128

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

// instantiate on-chip memory
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

// 4-bit index into the palette
logic [1:0] pixel_data;

// Palette registers
logic [3:0] [11:0] palette;

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

	// Set the next colors to display on-screen
	if (pixel_data != 0) begin
		red_next   = color[11:8] & {4{blank}};
		green_next = color[7:4]  & {4{blank}};
		blue_next  = color[3:0]  & {4{blank}};
	end else begin
		if ((x+1)%800 < `SIDEBAR_WIDTH) begin
			red_next   = 4'h3; 				// Dark discord grey
			green_next = 4'h3;
			blue_next  = 4'h3;
		end else begin
			red_next   = 4'h4 & {4{blank}}; // Light discord grey
			green_next = 4'h4 & {4{blank}};
			blue_next  = 4'h4 & {4{blank}};
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
	if(AVL_CS & AVL_WRITE & (AVL_ADDR > 16'h4AFF)) begin
		palette[AVL_ADDR[1:0]] <= AVL_WRITEDATA[11:0];
	end
end

// Pixel drawing
always_ff @ (posedge pixel_clk) 
begin
	red <= red_next;
	green <= green_next;
	blue <= blue_next;
end

endmodule
