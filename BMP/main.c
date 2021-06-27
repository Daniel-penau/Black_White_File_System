#include<stdio.h>
#include"imgreader.h"
int main() {
	printf("%d",sizeof(unsigned short));
	printf("%d",sizeof(unsigned int));
	printf("%d",sizeof(unsigned long));
	printf("%d",sizeof(unsigned char));
	FILE* fp = openBmpImage("test.bmp","r");
	BITMAPFILEHEADER* fileHead = readBmpFileHead(fp);
	BITMAPINFOHEADER* infoHead = readBmpInfoHead(fp);
	RGBDATA ** data = readBmpDataToArr(fp);
	//Be careful to avoid subscripts out of bounds
	for (int i = 0; i < 512; i++) {
		for (int j = 0; j < 512; j++) {
			 printf("The first (%d,%d) pixel: [%d,%d,%d] \n ", 511-i,j+1,data[i][j].blue, data[i][ j].green, data[i][j].red);
		}
		printf("\n");
	}
	closeBmpImage(fp);
	getchar();
	return 0;
}
