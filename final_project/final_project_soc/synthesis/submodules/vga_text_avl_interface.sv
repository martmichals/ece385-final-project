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
//`define NUM_REGS 601 //80*30 characters / 4 characters per register
//`define CTRL_REG 600 //index of control register

// `define TEXT_BIT rom_data[7 - (x % 8)]

// `define INVERT ((LOCAL_REG[(y >> 4) * 20 + (x >> 5)] >> (((x >> 3)%4)<<3))>>7)

// `define FGD_R (LOCAL_REG[CTRL_REG] >> 21) & 4'hf
// `define FGD_G (LOCAL_REG[CTRL_REG] >> 17) & 4'hf
// `define FGD_B (LOCAL_REG[CTRL_REG] >> 13) & 4'hf

// `define BKG_R (LOCAL_REG[CTRL_REG] >> 9) & 4'hf
// `define BKG_G (LOCAL_REG[CTRL_REG] >> 5) & 4'hf
// `define BKG_B (LOCAL_REG[CTRL_REG] >> 1) & 4'hf

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
	input  logic [11:0] AVL_ADDR,			// Avalon-MM Address
	input  logic [31:0] AVL_WRITEDATA,		// Avalon-MM Write Data
	output logic [31:0] AVL_READDATA,		// Avalon-MM Read Data
	
	// Exported Conduit (mapped to VGA port - make sure you export in Platform Designer)
	output logic [3:0]  red, green, blue,	// VGA color channels (mapped to output pins in top-level)
	output logic hs, vs						// VGA HS/VS
);

// Define the register block serving as VRAM, 601 32-bit registers
//logic [31:0] LOCAL_REG[`NUM_REGS];

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
	.vs(vs),						// Vertical sync
	.pixel_clk(pixel_clk),		// Pulse for every pixel drawn
	.blank(blank),				// Indicates blank, non-drawable pixel
	.sync(),					// Not used
	.DrawX(x),					// X-coordinate
	.DrawY(y)					// Y-coordinate
);

// instantiate on-chip memory
ram ram(
	// Input
	.address_a(AVL_ADDR),
	.address_b(vram_read_addr),
	.byteena_a(AVL_BYTE_EN),
	.clock_a(CLK),
	.clock_b(pixel_clk),
	.data_a(AVL_WRITEDATA),
	.data_b(),
	.wren_a(AVL_CS & AVL_WRITE),
	.wren_b(1'b0),

	// Output
	.q_a(AVL_READDATA),
	.q_b(vram_data)
);

// Define relevant ROM signals
logic [7:0] rom_data;
logic [10:0] rom_addr;

// Instantiate ROM used to load characters into VRAM
font_rom font_bitmaps(
	.addr(rom_addr), // ROM address
	.data(rom_data)  // ROM value
);

// 16-bit pixel data
logic [15:0] pixel_data;
logic pixel_bit;

// palette registers
logic [15:0] [11:0] palette;

// vram read wires
logic [11:0] vram_read_addr;
logic [31:0] vram_data;

logic [31:0] vram_data_cache;

logic [9:0] x_adjusted;
logic [9:0] y_adjusted;

logic [11:0] fg_color;
logic [11:0] bg_color;

// registers for vga sync
logic [3:0] red_next;
logic [3:0] green_next;
logic [3:0] blue_next;

// background color registers
// logic [4:0] FGD_R;
// logic [4:0] FGD_G;
// logic [4:0] FGD_B;
// logic [4:0] BKG_R;
// logic [4:0] BKG_G;
// logic [4:0] BKG_B;

// color registers
// logic [3:0] red_next;
// logic [3:0] green_next;
// logic [3:0] blue_next;

// logic [4:0] text_data;
// logic [4:0] blank_data;

// Text bit 
//assign TEXT_INVERT_BIT = rom_data[7 - (x % 8)];  // ^ ((LOCAL_REG[(y>>4)*20+(x>>5)]>>(((x>>3)%4)<<3))>>7);

// VRAM interaction logic
always_comb begin
	// x and y adjusted are 4 bits ahead of x and y
	x_adjusted = (x + 4) % 800;
	y_adjusted = x_adjusted < 4 ? y+1 : y;
	
	vram_read_addr = (y_adjusted>>4)*40+(x_adjusted>>4);


	pixel_data = vram_data_cache>>(((x>>3)%2)<<4);
	rom_addr = (pixel_data[14:8]<<4)|(y%16);
	pixel_bit = rom_data[7 - (x % 8)];

	fg_color = palette[pixel_data[7:4]];
	bg_color = palette[pixel_data[3:0]];

	red_next = (pixel_bit ? fg_color[11:8] : bg_color[11:8]) & {4{blank}};
	green_next = (pixel_bit ? fg_color[7:4] : bg_color[7:4]) & {4{blank}};
	blue_next = (pixel_bit ? fg_color[3:0] : bg_color[3:0]) & {4{blank}};


	// Read logic
	// vram_read_next = LOCAL_REG[AVL_ADDR];

	// Writing bitmask
	// vram_write_bitmask = {
	// 	AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3], AVL_BYTE_EN[3],
	// 	AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2], AVL_BYTE_EN[2],
	// 	AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1], AVL_BYTE_EN[1],
	// 	AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0], AVL_BYTE_EN[0]
	// };

	// color logic
	// FGD_R = (LOCAL_REG[600] >> 21);
	// FGD_G = (LOCAL_REG[600] >> 17);
	// FGD_B = (LOCAL_REG[600] >> 13);
	// BKG_R = (LOCAL_REG[600] >> 9);
	// BKG_G = (LOCAL_REG[600] >> 5);
	// BKG_B = (LOCAL_REG[600] >> 1);

	// Character drawing logic
	// pixel_data = LOCAL_REG[(y>>4)*20+(x>>5)]>>(((x>>3)%4)<<3);
	// rom_addr = (pixel_data<<4)|(y%16);
	// pixel_bit = rom_data[7 - (x % 8)];

	// text_data = {blank & (pixel_bit ^ pixel_data[7]), blank & (pixel_bit ^ pixel_data[7]), blank & (pixel_bit ^ pixel_data[7]), blank & (pixel_bit ^ pixel_data[7])};
	// red_next = (text_data & FGD_R) | (~text_data & BKG_R);
	// green_next = (text_data & FGD_G) | (~text_data & BKG_G);
	// blue_next = (text_data & FGD_B) | (~text_data & BKG_B);

	// red_next = rom_data[7 - (x % 8)] << 3;
	// green_next = 4'h0;
	// blue_next = 4'h0;

	//red_next = (text_data & blank_data & FGD_R) | (~text_data & blank_data & BKG_R);
	//green_next = (text_data & blank_data & FGD_G) | (~text_data & blank_data & BKG_G);
	//blue_next = (text_data & blank_data & FGD_B) | (~text_data & blank_data & BKG_B);

	// red_next = blank ? 4'h0 : (TEXT_INVERT_BIT ? FGD_R : BKG_R);
	// green_next = blank ? 4'h0 : (TEXT_INVERT_BIT ? FGD_G : BKG_G);
	// blue_next = blank ? 4'h0 : (TEXT_INVERT_BIT ? FGD_B : BKG_B);	

end

always_ff @ (posedge CLK) begin

	if(x % 16 == 0) begin
		vram_data_cache <= vram_data;
	end

	// if loading palette
	// update palette registers
	if(AVL_CS & AVL_WRITE & AVL_ADDR[11]) begin

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

// Read and write from AVL interface to register block, note that READ waitstate = 1, so this should be in always_ff
//always_ff @ (posedge CLK) begin
    //AVL_READDATA <= vram_read_next;

	// if (AVL_CS & AVL_WRITE) begin
	// 	LOCAL_REG[AVL_ADDR] <= (AVL_WRITEDATA & vram_write_bitmask) | (LOCAL_REG[AVL_ADDR] & ~vram_write_bitmask); 

		// if (AVL_ADDR == 600) begin
		// 	FGD_R <= (LOCAL_REG[600] >> 21);
		// 	FGD_G <= (LOCAL_REG[600] >> 17);
		// 	FGD_B <= (LOCAL_REG[600] >> 13);
		// 	BKG_R <= (LOCAL_REG[600] >> 9);
		// 	BKG_G <= (LOCAL_REG[600] >> 5);
		// 	BKG_B <= (LOCAL_REG[600] >> 1);
		// end
	// end
//end


// Pixel drawing
always_ff @ (posedge pixel_clk) 
begin
	red <= red_next;
	green <= green_next;
	blue <= blue_next;


end

endmodule
