#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CT_SIZE 1024   // Bitmap color table size
#define HEADER_SIZE 54 // Bitmap file header size

typedef struct {
    unsigned char header[HEADER_SIZE];
    uint32_t width;
    uint32_t height;
    uint32_t bitDepth;
    bool CT_EXISTS;
    unsigned char colorTable[CT_SIZE];
    unsigned char *imageBuffer;
} bitmap;

// helper function, verify a filename ends with extension.
bool endsWith(char *str, const char *ext) {
    if (!str || !ext) {
        return false;
    }
    size_t len_str = strlen(str);
    size_t len_ext = strlen(ext);

    // Good check to make sure the strings weren't given reversed.
    if (len_ext > len_str) {
        return false;
    }

    return strcmp(str + len_str - len_ext, ext) == 0;
}

// free memory allocated for bitmap structs.
void freeImage(bitmap *bmp) {
    free(bmp->imageBuffer);
    bmp->imageBuffer == NULL; // Avoid dangling pointer.
}
// returns false early and prints an error message if operation not complete.
// returns true on success of the operation.
bool readImage(char *filename1, bitmap *bitmapIn) {

    FILE *streamIn = fopen(filename1, "rb");
    if (streamIn == NULL) {
        printf("Error opening file or file not found!\n");
        return false;
    }

    for (int i = 0; i < HEADER_SIZE; i++) {
        bitmapIn->header[i] = getc(streamIn);
    }

    // width starts at address of byte(char) 18, which is then cast to an
    // int*, so it can be dereferenced into an int, so it is cast to a 4
    // byte int instead stead of a single byte from the char header array.
    // Then the height can be retreived from the next 4 byts and so on.
    bitmapIn->width = *(int *)&bitmapIn->header[18];
    bitmapIn->height = *(int *)&bitmapIn->header[22];
    bitmapIn->bitDepth = *(int *)&bitmapIn->header[28];

    const size_t IMAGE_SIZE = bitmapIn->width * bitmapIn->height;

    // if the bit depth is less than or equal to 8 then we need to read the
    // color table. The read content is going to be stored in colorTable.
    // Not all bitmap images have color tables.
    if (bitmapIn->bitDepth <=
        8) { // by definition of bitmap, <= 8 has a color table
        bitmapIn->CT_EXISTS = true;
    }

    if (bitmapIn->CT_EXISTS) {
        fread(bitmapIn->colorTable, sizeof(char), CT_SIZE, streamIn);
    }
    bitmapIn->imageBuffer = (char *)calloc(IMAGE_SIZE, sizeof(char));
    if (bitmapIn->imageBuffer == NULL) {
        return false;
    }

    fread(bitmapIn->imageBuffer, sizeof(char), IMAGE_SIZE, streamIn);
    fclose(streamIn);
    return true;
}

void writeImage(char *filename, bitmap *bitmapIn) {
    FILE *streamOut = fopen(filename, "wb");
    printf("Filename2: %s\n", filename);

    fwrite(bitmapIn->header, sizeof(char), HEADER_SIZE, streamOut);

    if (bitmapIn->CT_EXISTS) {
        fwrite(bitmapIn->colorTable, sizeof(char), CT_SIZE, streamOut);
    }
    size_t imageSize = bitmapIn->width * bitmapIn->height;
    fwrite(bitmapIn->imageBuffer, sizeof(char), imageSize, streamOut);
    fclose(streamOut);
}

int main(int argc, char *argv[]) {

    char *filename1 = NULL;
    char *filename2 = NULL;
    bool freeFilename2 = false; // this may or may not be dynamically allocated.
    const char *suffix = "_copy";
    const char *extension = ".bmp";

    if (argc > 1) {
        filename1 = argv[1];
        printf("Filename1: %s\n", filename1);

        // confirm filename1 ends with extension
        if (!endsWith(filename1, extension)) {
            printf("%s does not end with %s", filename1, extension);
            return -1;
        }
    }

    // If second filename exists from argv point to that.
    if (argc > 2) {
        filename2 = argv[2];
        // confirm filename2 ends with extension
        if (!endsWith(filename2, extension)) {
            printf("%s does not end with %s", filename2, extension);
            return -1;
        }
    }
    // else create a filename based on the first and allocate memory for the
    // new name.

    else {
        // Finds the last position of the  '.' in the filename
        char *dot_pos = strrchr(filename1, '.');
        if (dot_pos == NULL) {
            printf("\".\" not found in filename.\n");
            return -1;
        }

        // Calculate the length of the parts
        size_t base_len = dot_pos - filename1;
        size_t suffix_len = strlen(suffix);
        size_t extention_len = strlen(extension);

        filename2 = (char *)calloc(base_len + suffix_len + extention_len + 1,
                                   sizeof(char));
        if (filename2 == NULL) {
            printf("Memory allocation for output filename has failed.\n");
            return -1;
        }
        freeFilename2 = true;
        // Copy the base part of filename1 and append the suffix and extension.
        // strncpy copies the first base_len number of chars from filename1 into
        // filename2
        strncpy(filename2, filename1, base_len);
        // use ptr math to copy suffix to filename2ptr's + position + (can't use
        // strcat because strncpy doesn't null terminate.)
        strcpy(filename2 + base_len, suffix);
        strcpy(filename2 + base_len + suffix_len, extension);
    }

    bitmap bitmapIn = {.header = {0},
                       .width = 0,
                       .height = 0,
                       .bitDepth = 0,
                       .CT_EXISTS = false,
                       .colorTable = {0},
                       .imageBuffer = NULL};

    bool imageRead = readImage(filename1, &bitmapIn);
    if (!imageRead) {
        printf("Image read failed.\n");
        return -1;
    }

    writeImage(filename2, &bitmapIn);

    if (freeFilename2 && filename2 != NULL) {
        free(filename2);
        filename2 = NULL;
        freeFilename2 = false;
    }

    freeImage(&bitmapIn);

    printf("width: %d\n", bitmapIn.width);
    printf("height: %d\n", bitmapIn.height);
    printf("bitDepth: %d\n", bitmapIn.bitDepth);

    return 0;
}