#pragma once

// Lookup table for expanding 5-bit color values to 8-bit (0-31 -> 0-255).
// In DXT compression, colors are stored using 5 bits for red and blue channels (0-31).
// This table maps each 5-bit value to its corresponding 8-bit representation 
// by scaling it up to the full 0-255 range.
static constexpr uint8_t Expand5[32] = {  // 5-bit to 8-bit conversation
    0, 8, 16, 24, 32, 41, 49, 57, 65, 74, 82, 90, 98, 106, 115, 123,
    131, 139, 148, 156, 164, 172, 180, 189, 197, 205, 213, 222, 230, 238, 246, 255
};

// Lookup table for expanding 6-bit color values to 8-bit (0-63 -> 0-255).
// In DXT compression, the green channel is stored using 6 bits (0-63).
// This table maps each 6-bit value to its corresponding 8-bit representation
// by scaling it up to the full 0-255 range.
static constexpr uint8_t Expand6[64] = {  // 6-bit to 8-bit conversion
    0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
    64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124,
    128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
    192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252
};

// Bit shift values for extracting 3-bit alpha indices from the 48-bit alpha block in DXT5.
// Each pixel in a 4×4 block has a 3-bit alpha index, stored sequentially in a 48-bit field (6 bytes).
// The shift values determine where each pixel's alpha index starts in the 64-bit variable storing these 48 bits.
static constexpr int AlphaShift[16] = {
     0,  3,  6,  9, // Row 0 (left to right)
    12, 15, 18, 21, // Row 1
    24, 27, 30, 33, // Row 2
    36, 39, 42, 45  // Row 3
};
// Bit shift values for extracting 2-bit color indices from the 32-bit color block in DXT5.
// Each pixel in a 4×4 block has a 2-bit color index, stored sequentially in a 32-bit field (4 bytes).
// The shift values indicate where each pixel's color index starts in the 32-bit integer.
static constexpr int ColorShift[16] = {
     0,  2,  4,  6,   // Row 0 (left to right)
     8, 10, 12, 14,   // Row 1
    16, 18, 20, 22,   // Row 2
    24, 26, 28, 30    // Row 3
};

void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	uint32_t* image);

void DecompressBlockDXT3(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	uint32_t* image);

void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	uint32_t* image);

