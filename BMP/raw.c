#include <stdio.h>





 

// An unsigned char can store 1 Bytes (8bits) of data (0-255)

typedef unsigned char BYTE;

 

// Get the size of a file

long getFileSize(FILE *file)

{

    long lCurPos, lEndPos;

    lCurPos = ftell(file);

    fseek(file, 0, 2);

    lEndPos = ftell(file);

    fseek(file, lCurPos, 0);

    return lEndPos;

}

 

int main()

{

    const char *filePath = "./test.bmp"; 

    BYTE *fileBuf;          // Pointer to our buffered data

    FILE *file = NULL;      // File pointer

 

    // Open the file in binary mode using the "rb" format string

    // This also checks if the file exists and/or can be opened for reading correctly

    if ((file = fopen(filePath, "rb")) == NULL)

        printf( "Could not open specified file");

    else

        printf("File opened successfully");

 

    // Get the size of the file in bytes

    long fileSize = getFileSize(file);

 

    // Allocate space in the buffer for the whole file

    fileBuf = malloc(sizeof(BYTE[fileSize]));

 

    // Read the file in to the buffer

    fread(fileBuf, fileSize, 1, file);

 

    // Now that we have the entire file buffered, we can take a look at some binary infomation

   // Lets take a look in hexadecimal

    for (int i = 0; i < 100; i++)

        printf("%X", fileBuf[i]);

 

    getc(stdin);

    free(fileBuf);

        fclose(file);   // Almost forgot this

    return 0;

}

