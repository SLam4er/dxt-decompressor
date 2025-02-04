#include <stdint.h>
#include <stddef.h>
#include "decompress.h"
/*
DXT1/DXT3/DXT5 texture decompression

The original code is from Benjamin Dobell, see below for details. Compared to
the original this one adds DXT3 decompression, is valid C89, and is x64 
compatible as it uses fixed size integers everywhere. It also uses a different
PackRGBA order.

======

This version has been rewritten in C++, further optimized  and improved by AndraMidoxXx, 
who focused on increasing the performance of texture decompression on real-time
rendering.

---

Copyright (c) 2012, Matth�us G. "Anteru" Chajdas (http://anteru.net)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

---

Copyright (C) 2009 Benjamin Dobell, Glass Echidna

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

---
*/
static inline uint32_t PackRGBA (uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return r | (g << 8) | (b << 16) | (a << 24);
}

static void DecompressBlockDXT1Internal (const uint8_t* block,
	uint32_t* output,
	uint32_t outputStride,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	const uint8_t* alphaValues)
{

	uint16_t color0 = *reinterpret_cast<const uint16_t*>(block);
    	uint16_t color1 = *reinterpret_cast<const uint16_t*>(block + 2);
    	uint32_t code = *reinterpret_cast<const uint32_t*>(block + 4);

    	uint8_t r0 = Expand5[color0 >> 11];
    	uint8_t g0 = Expand6[(color0 >> 5) & 0x3F];
    	uint8_t b0 = Expand5[color0 & 0x1F];

    	uint8_t r1 = Expand5[color1 >> 11];
    	uint8_t g1 = Expand6[(color1 >> 5) & 0x3F];
    	uint8_t b1 = Expand5[color1 & 0x1F];

    	uint32_t colors[4];
    	colors[0] = PackRGBA(r0, g0, b0, 255);
    	colors[1] = PackRGBA(r1, g1, b1, 255);

    	if (color0 > color1) {
        	colors[2] = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, 255);
        	colors[3] = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, 255);
    	}
	else {
        	colors[2] = PackRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, 255);
        	colors[3] = transparent0 ? PackRGBA(0, 0, 0, 0) : PackRGBA(0, 0, 0, 255);
    	}

    	for (int j = 0; j < 4; ++j) {
        	for (int i = 0; i < 4; ++i) {
            		uint8_t alpha = alphaValues[j * 4 + i];
            		uint32_t finalColor = colors[(code >> (2 * (4 * j + i))) & 0x03];

            		if (alpha == 0) *simpleAlpha = 1;
            		else if (alpha < 255) *complexAlpha = 1;

            		output[j * outputStride + i] = (finalColor & 0xFFFFFF) | (alpha << 24);
        	}
    	}

}

static void DecompressBlockDXT1InternalRGB(const uint8_t* block,
		 uint8_t* output,
		uint32_t outputStride)
{

    	uint16_t color0 = *reinterpret_cast<const uint16_t*>(block);
    	uint16_t color1 = *reinterpret_cast<const uint16_t*>(block + 2);
    	uint32_t code = *reinterpret_cast<const uint32_t*>(block + 4);

    	uint8_t r0 = Expand5[color0 >> 11];
    	uint8_t g0 = Expand6[(color0 >> 5) & 0x3F];
    	uint8_t b0 = Expand5[color0 & 0x1F];

    	uint8_t r1 = Expand5[color1 >> 11];
    	uint8_t g1 = Expand6[(color1 >> 5) & 0x3F];
    	uint8_t b1 = Expand5[color1 & 0x1F];

    	uint8_t colors[4][3];  //(R, G, B)
    	colors[0][0] = r0; colors[0][1] = g0; colors[0][2] = b0;
    	colors[1][0] = r1; colors[1][1] = g1; colors[1][2] = b1;

    	if (color0 > color1) {
        	colors[2][0] = (2 * r0 + r1) / 3; colors[2][1] = (2 * g0 + g1) / 3; colors[2][2] = (2 * b0 + b1) / 3;
        	colors[3][0] = (r0 + 2 * r1) / 3; colors[3][1] = (g0 + 2 * g1) / 3; colors[3][2] = (b0 + 2 * b1) / 3;
    	}
	else {
        	colors[2][0] = (r0 + r1) / 2; colors[2][1] = (g0 + g1) / 2; colors[2][2] = (b0 + b1) / 2;
        	colors[3][0] = colors[3][1] = colors[3][2] = 0;
    	}

    	for (int j = 0; j < 4; ++j) {
        	for (int i = 0; i < 4; ++i) {
            		uint8_t* pixel = output + (j * outputStride + i) * 3;
            		uint8_t idx = (code >> (2 * (4 * j + i))) & 0x03;

            		pixel[0] = colors[idx][0];  // R
            		pixel[1] = colors[idx][1];  // G
            		pixel[2] = colors[idx][2];  // B
        	}
    	}
}


/*
void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t width: 				width of the texture being decompressed.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/ 
void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	uint32_t* image)
{

	static const uint8_t const_alpha [] = {
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255,
		255, 255, 255, 255
	};


	if( transparent0 )
		DecompressBlockDXT1Internal (blockStorage,
			image + x + (y * width), width, transparent0, simpleAlpha, complexAlpha, const_alpha);
	else
		DecompressBlockDXT1InternalRGB(blockStorage, ((uint8_t*)image) + x*3 + (y*3 * width), width);
}

/*
void DecompressBlockDXT5(): Decompresses one block of a DXT5 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t width: 				width of the texture being decompressed.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/ 

void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width,
                           const uint8_t* blockStorage,
                           int /*transparent0*/, int* simpleAlpha, int* complexAlpha,
                           uint32_t* image)
{
    	const uint8_t alpha0 = blockStorage[0];
    	const uint8_t alpha1 = blockStorage[1];

    	uint64_t alphaBits =  (uint64_t(blockStorage[2])  ) |
                         ((uint64_t(blockStorage[3])  ) << 8) |
                         ((uint64_t(blockStorage[4])  ) << 16) |
                         ((uint64_t(blockStorage[5])  ) << 24) |
                         ((uint64_t(blockStorage[6])  ) << 32) |
                         ((uint64_t(blockStorage[7])  ) << 40);

    	uint8_t alphaTable[8];
    	alphaTable[0] = alpha0;
    	alphaTable[1] = alpha1;

    	if (alpha0 > alpha1) {
        	alphaTable[2] = static_cast<uint8_t>((6 * alpha0 + 1 * alpha1) / 7);
        	alphaTable[3] = static_cast<uint8_t>((5 * alpha0 + 2 * alpha1) / 7);
        	alphaTable[4] = static_cast<uint8_t>((4 * alpha0 + 3 * alpha1) / 7);
        	alphaTable[5] = static_cast<uint8_t>((3 * alpha0 + 4 * alpha1) / 7);
        	alphaTable[6] = static_cast<uint8_t>((2 * alpha0 + 5 * alpha1) / 7);
        	alphaTable[7] = static_cast<uint8_t>((1 * alpha0 + 6 * alpha1) / 7);
    	}
	else {
        	alphaTable[2] = static_cast<uint8_t>((4 * alpha0 + 1 * alpha1) / 5);
        	alphaTable[3] = static_cast<uint8_t>((3 * alpha0 + 2 * alpha1) / 5);
        	alphaTable[4] = static_cast<uint8_t>((2 * alpha0 + 3 * alpha1) / 5);
        	alphaTable[5] = static_cast<uint8_t>((1 * alpha0 + 4 * alpha1) / 5);
        	alphaTable[6] = 0;
        	alphaTable[7] = 255;
    	}

    	const uint16_t color0 = *(const uint16_t*)(blockStorage + 8);
    	const uint16_t color1 = *(const uint16_t*)(blockStorage + 10);

    	const uint8_t r0 = Expand5[color0 >> 11];
    	const uint8_t g0 = Expand6[(color0 >> 5) & 0x3F];
    	const uint8_t b0 = Expand5[color0 & 0x1F];

    	const uint8_t r1 = Expand5[color1 >> 11];
    	const uint8_t g1 = Expand6[(color1 >> 5) & 0x3F];
    	const uint8_t b1 = Expand5[color1 & 0x1F];

    	const uint8_t rLerp0 = static_cast<uint8_t>((2 * r0 + r1) / 3);
    	const uint8_t gLerp0 = static_cast<uint8_t>((2 * g0 + g1) / 3);
    	const uint8_t bLerp0 = static_cast<uint8_t>((2 * b0 + b1) / 3);

    	const uint8_t rLerp1 = static_cast<uint8_t>((r0 + 2 * r1) / 3);
    	const uint8_t gLerp1 = static_cast<uint8_t>((g0 + 2 * g1) / 3);
    	const uint8_t bLerp1 = static_cast<uint8_t>((b0 + 2 * b1) / 3);

    	const uint32_t baseColors[4] = {
    		static_cast<uint32_t>(r0) | (static_cast<uint32_t>(g0) << 8) | (static_cast<uint32_t>(b0) << 16),
    		static_cast<uint32_t>(r1) | (static_cast<uint32_t>(g1) << 8) | (static_cast<uint32_t>(b1) << 16),
    		static_cast<uint32_t>(rLerp0) | (static_cast<uint32_t>(gLerp0) << 8) | (static_cast<uint32_t>(bLerp0) << 16),
    		static_cast<uint32_t>(rLerp1) | (static_cast<uint32_t>(gLerp1) << 8) | (static_cast<uint32_t>(bLerp1) << 16)
	};

    	const uint32_t code = *(const uint32_t*)(blockStorage + 12);

    	for (int pix = 0; pix < 16; ++pix) {
        	const int aShift = AlphaShift[pix];
        	const int aCode = (alphaBits >> aShift) & 0x07;
        	const uint8_t finalAlpha = alphaTable[aCode];

        	const int cShift = ColorShift[pix];
        	const int colorCode = (code >> cShift) & 0x03;
		const uint32_t finalColor = baseColors[colorCode] | (static_cast<uint32_t>(finalAlpha) << 24);

        	*simpleAlpha |= (finalAlpha == 0);
        	*complexAlpha |= (finalAlpha > 0 && finalAlpha < 255);

        	// (pix = 4 * j + i; j = pix / 4, i = pix % 4)
        	const int j = pix >> 2;
        	const int i = pix & 3;
        	image[i + x + (y + j) * width] = finalColor;
    	}
}

/*void DecompressBlockDXT3(): Decompresses one block of a DXT3 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:                                             x-coordinate of the first pixel in the block.
uint32_t y:                                             y-coordinate of the first pixel in the block.
uint32_t height:                                height of the texture being decompressed.
const uint8_t *blockStorage:    pointer to the block to decompress.
uint32_t *image:                                pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockDXT3(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	int transparent0, int* simpleAlpha, int *complexAlpha,
	uint32_t* image)
{
    	uint8_t alphaValues[16];
	int i;
	        for (i = 0; i < 4; ++i) {
                const uint16_t* alphaData = (const uint16_t*) (blockStorage);

                alphaValues [i*4 + 0] = (((*alphaData) >> 0) & 0xF ) * 17;
                alphaValues [i*4 + 1] = (((*alphaData) >> 4) & 0xF ) * 17;
                alphaValues [i*4 + 2] = (((*alphaData) >> 8) & 0xF ) * 17;
                alphaValues [i*4 + 3] = (((*alphaData) >> 12) & 0xF) * 17;

                blockStorage += 2;
        }

	DecompressBlockDXT1Internal (blockStorage,
		image + x + (y * width), width, transparent0, simpleAlpha, complexAlpha, alphaValues);
}
