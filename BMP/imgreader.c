
#define _CRT_SECURE_NO_DEPRECATE
#include"imgreader.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
 
 
FILE* openBmpImage(char* fileName, char* mode) {
	FILE* fp;
	if (strcmp(mode, "r") == 0) {
		mode = "rb";
	}
	else if (strcmp(mode,"w") == 0) {
		mode = "ab";
	}
	else {
		 //Output error message
		 fprintf(stderr,"File open mode: %s use error\n",mode);
		 //Failed to open file, return null pointer
		return NULL;
	}
	if ((fp = fopen(fileName,mode)) == NULL) {
		 fprintf(stderr, "Opening file: %s failed\n", fileName);
		return NULL;	
	}
	return fp;
}
 
void closeBmpImage(FILE* fp) {
	 //Close the file
	fclose(fp);
	 //printf("closed file\n");
	 //Release the file pointer
	free(fp);
	 //printf("File pointer released\n");
}
 
BITMAPFILEHEADER* readBmpFileHead(FILE* fp) {
	 //printf("%d\n", sizeof(BITMAPFILEHEADER));//This size is 16Bytes right
	BITMAPFILEHEADER* fileHead = (BITMAPFILEHEADER*)malloc(sizeof(BITMAPFILEHEADER));
	if (fileHead == NULL) {
		 fprintf(stderr, "Memory allocation failed");
	}
	if (fread(fileHead, sizeof(BITMAPFILEHEADER), 1, fp) != 1) {
		 fprintf(stderr, "Failed to read file header");
	}
	return fileHead;
}
 
 RGBDATA** createMatrix(int width,int height) {
	 //Create a two-dimensional array dynamically
	RGBDATA** Matrix;
	int i;
	
	Matrix = (RGBDATA **)malloc(sizeof(RGBDATA*) * height);
	if (Matrix == NULL) {
		 fprintf(stderr, "Memory allocation failed");
		return NULL;
	}
	
	for (i = 0; i < height; i++) {
		Matrix[i] = (RGBDATA *)malloc(sizeof(RGBDATA) * width);
		if (Matrix[i] == NULL) {
			 fprintf(stderr, "Memory allocation failed");
			return NULL;
		}
	}
	return(Matrix);
}
 
BITMAPINFOHEADER* readBmpInfoHead(FILE* fp) {
	//printf("%d\n", sizeof(BITMAPINFOHEADER));
	BITMAPINFOHEADER* infoHead = (BITMAPINFOHEADER*)malloc(sizeof(BITMAPINFOHEADER));
	if (infoHead == NULL) {
		 fprintf(stderr, "Memory allocation failed");
	}
	if (fread(infoHead, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
		 fprintf(stderr, "Failed to read the header");
	}
	 printf("Information header size: %d bytes\n", infoHead->bHeaderSize);
	 printf("Image width: %d pixels\n", infoHead->bImageWidth);
	 printf("Image height: %d pixels\n", infoHead->bImageHeight);
	 printf("Color bits: %d bits\n", infoHead->bBitsPerPixel);
	 printf("Horizontal pixels per meter: %d\n", infoHead->bXpixelsPerMeter);
	 printf("Vertical pixels per meter: %d\n", infoHead->bYpixelsPerMeter);
	 printf("Data block size: %d bytes\n", infoHead->bImageSize);
	 printf("Number of planes: %d\n", infoHead->bPlanes);
	 printf("Total number of colors used: %d\n", infoHead->bTotalColors);
	 printf("Total number of important colors: %d\n", infoHead->bImportantColors);
	 printf("Compression algorithm: %d\n", infoHead->bCompression);
	
	return infoHead;
}
 
RGBDATA** readBmpDataToArr(FILE* fp) {
	int i = 0, j = 0;
	int width = 0, height = 0;
	BITMAPFILEHEADER* fileHead = readBmpFileHead(fp);
	BITMAPINFOHEADER* infoHead = readBmpInfoHead(fp);
	width = infoHead->bImageWidth;
	height = infoHead->bImageHeight;
	RGBDATA** data = createMatrix(width,height);
	 //The palette is valid if the number of digits is less than
	if (infoHead->bBitsPerPixel < 8) {
		RGBQUAD* rgbQuad = (RGBQUAD*)malloc(sizeof(RGBQUAD));
		if(rgbQuad == NULL){
			 printf("Memory allocation failed");
		}
		if (fread(rgbQuad, sizeof(rgbQuad), 1, fp) != 1) {
			 printf("Failed to read palette");
		}
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fread(&data[i][j], sizeof(RGBDATA), 1, fp);
		}
	}
	return data;
}
 
