#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        close_encode_files(encInfo);
        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        close_encode_files(encInfo);
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Close all the files after successfully encoding or in each failure terminations */
Status close_encode_files(EncodeInfo *encInfo)
{
    if (encInfo->fptr_src_image)
        fclose(encInfo->fptr_src_image);
    if (encInfo->fptr_secret)
        fclose(encInfo->fptr_secret);
    if (encInfo->fptr_stego_image)
        fclose(encInfo->fptr_stego_image);
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // validate whether user has provided the .bmp or not
    if (argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // validate whether user has provided the .txt ot not
    if (argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    // fetch optional file
    if (argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }
    return e_success;
}

uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    printf("Size: %ld\n", ftell(fptr_secret));
    return ftell(fptr_secret);
}

Status check_capacity(EncodeInfo *encInfo)
{
    // get image size
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    // get secret file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if (encInfo->image_capacity > ((54 + 2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8))
        return e_success;
    else
        return e_failure;
}

Status copy_bmp_header(FILE *fptr_src, FILE *fptr_stego)
{
    char header[54];
    fseek(fptr_src, 0, SEEK_SET);
    fread(header, 54, sizeof(char), fptr_src);
    fwrite(header, 54, sizeof(char), fptr_stego);
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char mask = 1 << 7;
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_data_to_image(const char *data, int size, FILE *fptr_src, FILE *fptr_stego, EncodeInfo *encInfo)
{
    for (int i = 0; i < size; i++)
    {
        // read 8 bytes from beautiful.bmp
        fread(encInfo->image_data, 8, sizeof(char), fptr_src);
        // call encode_lsb_to_byte to encode the data to lsb
        encode_byte_to_lsb(data[i], encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), fptr_stego);
    }
    return e_success;
}

Status encode_magic_string(const char *magic_str, EncodeInfo *encInfo)
{
    // all the character encoding will call encode_data_to_image
    encode_data_to_image(magic_str, strlen(magic_str), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for (int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
    return e_success;
}

Status encode_size(uint size, FILE *fptr_src, FILE *fptr_stego)
{
    char rgb[32];
    // read 32 bytes from the source image
    fread(rgb, 32, sizeof(char), fptr_src);
    // encode_size_to_lsb to encode the integer into lsb of image data 
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, 32, sizeof(char), fptr_stego);
    return e_success;
}

Status encode_secret_file_extn(const char *ext, EncodeInfo *encInfo)
{
    // call encode_data_to_image
    encode_data_to_image(ext, strlen(ext), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char rgb[32];
    // read 32 bytes from the source image
    fread(rgb, 32, sizeof(char), encInfo->fptr_src_image);
    // encode_size_to_lsb to encode the integer into lsb of image data 
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, 32, sizeof(char), encInfo->fptr_stego_image);
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;
    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        // read a character from secret file
        fread(&ch, 1, sizeof(char), encInfo->fptr_secret);
        // read 8 bytes of rgb from image file
        fread(encInfo->image_data, 8, sizeof(char), encInfo->fptr_src_image);
        // call encode_byte_to_lsb to encode the secret data
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, 8, sizeof(char), encInfo->fptr_stego_image);
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_stego)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_stego);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    // encoding function
    if (open_files(encInfo) == e_success)
    {
        printf("Opened all the files successfully\n");
        if (check_capacity(encInfo) == e_success)
        {
            fseek(encInfo->fptr_secret, 0, SEEK_SET);
            printf("Image have enough capacity to encode\n");
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Successfully copied the header\n");
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Successfully encoded the magic string\n");
                    if (encode_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Successfully encoded secret file extention size\n");
                        if (encode_secret_file_extn(".txt", encInfo) == e_success)
                        {
                            printf("Successfully encoded secret file extension\n");
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Successfully encoded secret file size\n");
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Successfully encoded the secret data\n");
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Successfully copied the remaining RGB data\n");
                                    }
                                    else
                                    {
                                        printf("Failed to copy remaining RGB data\n");
                                        close_encode_files(encInfo);
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode the secret data\n");
                                    close_encode_files(encInfo);
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode the secret file size\n");
                                close_encode_files(encInfo);
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode the secret file extension\n");
                            close_encode_files(encInfo);
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to encode the secret file extention size\n");
                        close_encode_files(encInfo);
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode the magic string\n");
                    close_encode_files(encInfo);
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy the header\n");
                close_encode_files(encInfo);
                return e_failure;
            }
        }
        else
        {
            printf("Failure! Image does not have enough capacity\n");
            close_encode_files(encInfo);
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open the file\n");
        close_encode_files(encInfo);
        return e_failure;
    }
    close_encode_files(encInfo);
    return e_success;
}