
#pragma once
 
#include<stdio.h>
typedef unsigned int DWORD;  // 4bytes
typedef unsigned short WORD;  // 2bytes
typedef signed long LONG;  // 4bytes
typedef unsigned char BYTE;  // 1bytes
 
 
#pragma pack(push)
 #pragma pack(1)// Modify the default alignment value
 /*Bitmap file header structure*/
typedef struct tagBITMAPFILEHEADER {
	WORD bFileType;
	DWORD bFileSize;
	WORD bReserved1;
	WORD bReserved2;
	DWORD bPixelDataOffset;
}BITMAPFILEHEADER; //14bytes
#pragma pack(pop)
 
 /*Bitmap file information header structure*/
typedef struct tagBITMAPINFOHEADER {
	 DWORD bHeaderSize; // Total size of image information header (40bytes)
	 LONG bImageWidth; // Image width (pixels)
	 LONG bImageHeight; // Image height
	 WORD bPlanes; // should be 0
	 WORD bBitsPerPixel; // Number of pixels
	 DWORD bCompression; // Image compression method
	 DWORD bImageSize; // Image size (bytes)
	 LONG bXpixelsPerMeter; // Horizontal pixels per meter
	 LONG bYpixelsPerMeter; // Vertical pixels per meter
	 DWORD bTotalColors; // The total number of colors used, if the number of pixels is greater than 8, this field is meaningless
	 DWORD bImportantColors; // important color numbers, generally useless
}BITMAPINFOHEADER; //40bytes
 
 /*Bitmap file palette structure*/
typedef struct tagRGBQUAD {
	BYTE	rgbBlue;
	BYTE	rgbGreen;
	BYTE	rgbRed;
	BYTE	rgbReserved;
}RGBQUAD;
 
 /*Pixel RGB structure*/
typedef struct tagRGB {
	BYTE blue;
	BYTE green;
	BYTE red;
}RGBDATA;
