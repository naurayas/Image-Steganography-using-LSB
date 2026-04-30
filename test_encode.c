/* Name         : Naura Yasmeen U
   Reg No.      : 25017_094
   Start Date   : 19/11/2025
   End Date     : 22/11/2025
   Project Name : Steganography
   Description  : This project implements a steganography system using .bmp image files.
                  It allows user to hide secret data inside the image by modifying the
                  LSB bits of the pixel values, generating a new encoded image that looks 
                  identical to the original. The program can also be used to decode the hidden
                  information by extracting the embedded bits. This ensures security of data
                  transmission without altering the visible quality of the image.
*/



#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    // check the operation type (encoding/decoding)
    if (check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        printf("Selected Encoding\n");
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validate input arguments is sucessfull\n");
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Completed encoding\n");
            }
            else
            {
                printf("Failed to do the encoding\n");
            }
        }
        else
        {
            printf("Failed to validate the input arguments\n");
        }

    }
    else if (check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        printf("Selected Decoding\n");
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and validate input arguments is sucessfull\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Completed decoding\n");
            }
            else
            {
                printf("Failed to do the decoding\n");
            }
        }
        else
        {
            printf("Failed to validate the input arguments\n");
        }
    }
    else
    {
        printf("Invalid option\n");
        printf("--------------------------Usage-------------------------\n");
        printf("Encoding: ./stego -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding: ./stego -d stego.bmp decoded.txt\n");
    }

    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(argv[1] == NULL)
        return e_unsupported;
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}