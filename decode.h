#ifndef DECODE_H
#define DECODE_H
#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;
    uint ext_size;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Decoded File Info */
    char *decoded_fname;
    FILE *fptr_decoded;
    uint dec_secret_size;
    uint secret_file_size;
    char ext_data[MAX_FILE_SUFFIX];
    char decoded_data[MAX_SECRET_BUF_SIZE];

} DecodeInfo;

/* Decoding function prototypes */

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_to_decode(DecodeInfo *decInfo);

/* Decode magic string */
Status decode_magic_string(const char *magic_str, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_ext_size(DecodeInfo *decInfo);

/* function to decode size from lsb */
Status decode_size_from_lsb(char *size_buffer, DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_ext(DecodeInfo *decInfo);

/* function to decode bytes from lsb */
Status decode_bytes_from_lsb(char *data, char *data_buffer);

/* Deocode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data to .txt file */
Status decode_secret_data(DecodeInfo *decInfo);

/* Function to close all the files */
Status close_decode_files(DecodeInfo *decInfo);

#endif 