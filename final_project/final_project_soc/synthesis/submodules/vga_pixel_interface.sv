/************************************************************************
Avalon-MM Interface VGA Text mode display
Register Map:
0x000-0x0257 : VRAM, 80x30 (2400 byte, 600 word) raster order (first column then row)
0x258        : control register
VRAM Format:
X->
[ 31  30-24][ 23  22-16][ 15  14-8 ][ 7    6-0 ]
[IV3][CODE3][IV2][CODE2][IV1][CODE1][IV0][CODE0]
IVn = Draw inverse glyph
CODEn = Glyph code from IBM codepage 437
Control Register Format:
[[31-25][24-21][20-17][16-13][ 12-9][ 8-5 ][ 4-1 ][   0    ] 
[[RSVD ][FGD_R][FGD_G][FGD_B][BKG_R][BKG_G][BKG_B][RESERVED]
VSYNC signal = bit which flips on every Vsync (time for new frame), used to synchronize software
BKG_R/G/B = Background color, flipped with foreground when IVn bit is set
FGD_R/G/B = Foreground color, flipped with background when Inv bit is set
************************************************************************/

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
	input  logic [15:0] AVL_ADDR,			// Avalon-MM Address
	input  logic [31:0] AVL_WRITEDATA,		// Avalon-MM Write Data
	output logic [31:0] AVL_READDATA,		// Avalon-MM Read Data
	
	// Exported Conduit (mapped to VGA port - make sure you export in Platform Designer)
	output logic [3:0]  red, green, blue,	// VGA color channels (mapped to output pins in top-level)
	output logic hs, vs						// VGA HS/VS
);

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

// Define relevant ROM signals
logic [7:0] rom_data;
logic [10:0] rom_addr;

// Instantiate ROM used to load characters into VRAM
font_rom font_bitmaps(
	.addr(rom_addr), // ROM address
	.data(rom_data)  // ROM value
);

// 4-bit pixel data
logic [3:0] pixel_data;

// Palette registers
logic [15:0] [11:0] palette;

// VRAM read wires
logic [11:0] vram_read_addr;
logic [31:0] vram_data;

// Look-ahead cache data and pixel positions
logic [31:0] vram_data_cache;
logic [9:0] x_adjusted;
logic [9:0] y_adjusted;

// Foreground color
logic [11:0] fg_color;

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
	vram_read_addr = (y_adjusted*80)+(x_adjusted[9:3]);

	// Index of the pixel to draw into the color palette
	case (x[2:0])
		3'b000: pixel_data = vram_data_cache[31:28];
		3'b001: pixel_data = vram_data_cache[27:24];
		3'b010: pixel_data = vram_data_cache[23:20];
		3'b011: pixel_data = vram_data_cache[19:16];
		3'b100: pixel_data = vram_data_cache[15:12];
		3'b101: pixel_data = vram_data_cache[11:8];
		3'b110: pixel_data = vram_data_cache[7:4];
		3'b111: pixel_data = vram_data_cache[3:0];
		default: pixel_data = 4'b0000;
	endcase

	// Get the foreground and background colors from the palette
	fg_color = palette[pixel_data];

	// Set the next colors to display on-screen
	red_next   = fg_color[11:8] & {4{blank}};
	green_next = fg_color[7:4]  & {4{blank}};
	blue_next  = fg_color[3:0]  & {4{blank}};

	// Dummy read values
	AVL_READDATA = 32'h0000;
end

always_ff @ (posedge CLK) begin
	// Load the cache
	if(x % 8 == 0) begin
		vram_data_cache <= vram_data;
	end

	// if loading palette
	// update palette registers
	if(AVL_CS & AVL_WRITE & (AVL_ADDR > 16'h95FF)) begin

		// bits 7-0
		// AVL_WRITEDATA[0] unused
		if(AVL_BYTE_EN[0]) begin
			palette[AVL_ADDR[2:0]*2][6:0] <= AVL_WRITEDATA[7:1];
		end

		// bits 15-8
		if(AVL_BYTE_EN[1]) begin
			palette[AVL_ADDR[2:0]*2][11:7] <= AVL_WRITEDATA[12:8];
			palette[AVL_ADDR[2:0]*2+1][2:0] <= AVL_WRITEDATA[15:13];
		end

		// bits 23-16
		if(AVL_BYTE_EN[2]) begin
			palette[AVL_ADDR[2:0]*2+1][10:3] <= AVL_WRITEDATA[23:16];
		end

		// bits 31-24
		// AVL_WRITEDATA[31:25] unused
		if(AVL_BYTE_EN[3]) begin
			palette[AVL_ADDR[2:0]*2+1][11] <= AVL_WRITEDATA[24];
		end

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
