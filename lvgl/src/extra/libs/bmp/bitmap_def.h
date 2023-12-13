// C++ Programing Assignment1
#ifndef bitmap_def_h
#define bitmap_def_h

#pragma once
#pragma pack(push,1)

typedef struct{
    unsigned short type;			// Magic identifier
    unsigned int size;				// File size in bytes
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;			// Offset to image data,bytes
}BITMAP_HEADER;

typedef struct{
    unsigned int size;				// Header size in bytes
    int width;						// Width of image
    int height;						// Height of image
    unsigned short planes;			// Number of color planes
    unsigned short bits;			// Bits per pixel
    unsigned int compression;		// Compression type
    unsigned int image_size;		// Image size in bytes
    int x_resolution;				// X resolution
    int y_resolution;				// Y resolution
    unsigned int colors;			// Number of colors
    unsigned int important_colors;	// Important colors
}BITMAP_INFO_HEADER;

#pragma pack(pop)

#endif
