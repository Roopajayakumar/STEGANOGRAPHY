#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(argv[2] == NULL)
    {
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];

    if(argv[3] != NULL)
    {
        decInfo->output_fname = argv[3];
    }
    else
    {
        decInfo->output_fname = "output";
    }

    return e_success;
}
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open file %s\n",
               decInfo->stego_image_fname);
        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    if(decInfo->fptr_output == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to create file %s\n",
               decInfo->output_fname);
        return e_failure;
    }

    return e_success;
}
Status do_decoding(DecodeInfo *decInfo)
{
    printf("Decoding started\n");

    if(decode_magic_string(decInfo) == e_success)
    {
        printf("Magic string decoded successfully\n");
    }
    else
    {
        printf("Magic string decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_success)
    {
        printf("Secret file extension decoded successfully\n");
    }
    else
    {
        printf("Secret file extension decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_success)
    {
        printf("Secret file size decoded successfully\n");
    }
    else
    {
        printf("Secret file size decoding failed\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo) == e_success)
    {
        printf("Secret file data decoded successfully\n");
    }
    else
    {
        printf("Secret file data decoding failed\n");
        return e_failure;
    }

    return e_success;
}
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic[3];
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    for(int i = 0; i < 2; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        magic[i] = 0;

        for(int j = 0; j < 8; j++)
        {
            magic[i] = magic[i] | ((image_buffer[j] & 1) << j);
        }
    }

    magic[2] = '\0';

    printf("Decoded magic string = %s\n", magic);

    return e_success;
}
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char extn[5];

    for(int i = 0; i < 4; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        extn[i] = 0;

        for(int j = 0; j < 8; j++)
        {
            extn[i] = extn[i] | ((image_buffer[j] & 1) << j);
        }
    }

    extn[4] = '\0';

    printf("Decoded extension = %s\n", extn);

    return e_success;
}
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char size_data[4];
    long size = 0;

    for(int i = 0; i < 4; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        size_data[i] = 0;

        for(int j = 0; j < 8; j++)
        {
            size_data[i] = size_data[i] |
                           ((image_buffer[j] & 1) << j);
        }
    }

    size = ((unsigned char)size_data[0] << 24) |
           ((unsigned char)size_data[1] << 16) |
           ((unsigned char)size_data[2] << 8) |
           (unsigned char)size_data[3];

    printf("Decoded secret file size = %ld\n", size);
    decInfo->size_secret_file = size;

    return e_success;
}
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    for(long i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        ch = 0;

        for(int j = 0; j < 8; j++)
        {
            ch = ch | ((image_buffer[j] & 1) << j);
        }

        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}