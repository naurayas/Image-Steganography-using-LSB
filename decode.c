#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    /* check whether the user has given the .bmp file or not*/
    if (argv[2] != NULL && (strcmp(strstr(argv[2], "."), ".bmp") == 0))
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // fetch optional file
    if (argv[3] != NULL)
    {
        decInfo->decoded_fname = argv[3];
    }
    else
    {
        decInfo->decoded_fname = "decode.txt";
    }
    return e_success;
}

Status open_files_to_decode(DecodeInfo *decInfo)
{
    /* encoded file */
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    /* error handling */
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    /* decoded file */
    decInfo->fptr_decoded = fopen(decInfo->decoded_fname, "w");
    /* error handling */
    if (decInfo->fptr_decoded == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->decoded_fname);
        close_decode_files(decInfo);
        return e_failure;
    }
    return e_success;
}

/* Close all opened files if failure occurs or end of program */
Status close_decode_files(DecodeInfo *decInfo)
{
    if (decInfo->fptr_stego_image)
        fclose(decInfo->fptr_stego_image);
    if (decInfo->fptr_decoded)
        fclose(decInfo->fptr_decoded);
}

/* decode magic string */
Status decode_magic_string(const char *magic_str, DecodeInfo *decInfo)
{
    int len = strlen(magic_str);
    char magic[len + 1];
    for (int i = 0; i < len; i++)
    {
        char rgb[8];
        /* read 8 bytes of RGB from stego file */
        fread(rgb, 8, sizeof(char), decInfo->fptr_stego_image);
        unsigned char ch = 0;
        for (int bit = 0; bit < 8; bit++)
        {
            ch = (ch << 1) | (rgb[bit] & 1);
        }
        magic[i] = ch;
    }
    magic[len] = '\0';
    if (strcmp(magic, magic_str) == 0)
    {
        return e_success;
    }
    else
        return e_failure;
}

Status decode_size_from_lsb(char *size_buffer, DecodeInfo *decInfo)
{
    unsigned int ch = 0;
    for (int i = 0; i < 32; i++)
    {
        ch = (ch << 1) | (size_buffer[i] & 1);
    }
    decInfo->dec_secret_size = ch;
    return e_success;
}

/* decode the secret file extension size */
Status decode_ext_size(DecodeInfo *decInfo)
{
    char str[32];
    fread(str, 32, sizeof(char), decInfo->fptr_stego_image);
    decode_size_from_lsb(str, decInfo);
    decInfo->ext_size = decInfo->dec_secret_size;
    return e_success;
}

Status decode_bytes_from_lsb(char *decoded_buffer, char *image_data)
{
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
    {
        ch = (ch << 1) | (image_data[i] & 1);
    }
    *decoded_buffer = ch;
    return e_success;
}

/* decode secret file extension */
Status decode_secret_file_ext(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->ext_size; i++)
    {
        fread(decInfo->image_data, 8, sizeof(char), decInfo->fptr_stego_image);
        decode_bytes_from_lsb(&decInfo->ext_data[i], decInfo->image_data);
    }
    decInfo->ext_data[decInfo->ext_size] = '\0';
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char file_size[32];
    fread(file_size, 32, sizeof(char), decInfo->fptr_stego_image);
    decode_size_from_lsb(file_size, decInfo);
    decInfo->secret_file_size = decInfo->dec_secret_size;
    return e_success;
}

Status decode_secret_data(DecodeInfo *decInfo)
{
    for (int i = 0; i < decInfo->secret_file_size; i++)
    {
        fread(decInfo->image_data, 8, sizeof(char), decInfo->fptr_stego_image);
        decode_bytes_from_lsb(decInfo->decoded_data, decInfo->image_data);
        fwrite(&decInfo->decoded_data[0], 1, sizeof(char), decInfo->fptr_decoded);
    }
    return e_success;
}

/* Do decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    /* Open files */
    if (open_files_to_decode(decInfo) == e_success)
    {
        printf("Opened all files successfully\n");
        /* decode magic string */
        fseek(decInfo->fptr_stego_image, 54, SEEK_SET);
        if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            printf("Successfully decoded magic string\n");
            /* decode the secret file extension size */
            decode_ext_size(decInfo);
            if (decInfo->ext_size > 0)
            {
                printf("Succesfully decoded secret file extension size\n");
                /* print the extension size */
                printf("Decoded secret file extension size = %u\n", decInfo->ext_size);
                /* decode secret file extension */
                if (decode_secret_file_ext(decInfo) == e_success)
                {
                    printf("Successfully decoded secret file extension\n");
                    printf("Secret file extension = %s\n", decInfo->ext_data);
                    /* decode secret file size */
                    if (decode_secret_file_size(decInfo) == e_success)
                    {
                        printf("Successfully decoded the secret file size\n");
                        /* print the secret file size */
                        printf("Secret file size = %u\n", decInfo->secret_file_size);
                        if (decode_secret_data(decInfo) == e_success)
                        {
                            printf("Successfully decoded secret file data\n");
                        }
                        else
                        {
                            printf("Failed to decode secret file data\n");
                            close_decode_files(decInfo);
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to decode secret file size");
                        close_decode_files(decInfo);
                        return e_failure;
                    }

                }
                else
                {
                    printf("Failed to decode secret file extension\n");
                    close_decode_files(decInfo);
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to decode secret file extension size\n");
                close_decode_files(decInfo);
                return e_failure;
            }
        }
        else
        {
            printf("Failed to decode magic string\n");
            close_decode_files(decInfo);
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open files\n");
        close_decode_files(decInfo);
        return e_failure;
    }
    close_decode_files(decInfo);
    return e_success;
}