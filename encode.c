#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include<string.h>

/* Function Definitions */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(argv[2] == NULL)
    {
        return e_failure;
    }

    if(argv[3] == NULL)
    {
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    if(argv[4] != NULL)
    {
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }

    return e_success;
}

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
uint get_file_size(FILE *fptr)
{
    long current_position;
    long size;

    current_position = ftell(fptr);

    fseek(fptr, 0, SEEK_END);

    size = ftell(fptr);

    fseek(fptr, current_position, SEEK_SET);

    return size;
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if(encInfo->image_capacity >= (encInfo->size_secret_file * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];

    fseek(fptr_src_image, 0, SEEK_SET);

    fread(header, 54, 1, fptr_src_image);

    fwrite(header, 54, 1, fptr_dest_image);

    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    if(copy_bmp_header(encInfo->fptr_src_image,
                       encInfo->fptr_stego_image) == e_success)
    {
        printf("BMP header copied successfully\n");
        if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
        {
            char *extension = strrchr(encInfo->secret_fname, '.');
            if(extension != NULL)
            {
                printf("Extension = %s\n", extension);
                if(encode_secret_file_extn(extension, encInfo) == e_success)
                {
                    printf("Secret file extension encoded successfully\n");
                    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                    {
                        printf("Secret file size encoded successfully\n");
                        if(encode_secret_file_data(encInfo) == e_success)
                        {
                            printf("Secret file data encoded successfully\n");
                        }
                        else
                        {
                            printf("Secret file data encoding failed\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Secret file size encoding failed\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Secret file extension encoding failed\n");
                    return e_failure;
                }
                if(copy_remaining_img_data(encInfo->fptr_src_image,
                           encInfo->fptr_stego_image) == e_success)
                {



                    
                    printf("Remaining image data copied successfully\n");
                }
                else
                {
                    printf("Failed to copy remaining image data\n");
                    return e_failure;
                }
            }
            printf("Magic string encoded successfully\n");
        }
        else
        {
            printf("Magic string encoding failed\n");
            return e_failure;
        }
        return e_success;
    }
    else
    {
        printf("Failed to copy BMP header\n");
        return e_failure;
    }
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    return encode_data_to_image(magic_string,
                                2,
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image);
}
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char image_buffer[8];

    for(int i = 0; i < size; i++)
    {
        fread(image_buffer, 8, 1, fptr_src_image);

        encode_byte_to_lsb(data[i], image_buffer);

        fwrite(image_buffer, 8, 1, fptr_stego_image);
    }

    return e_success;
}
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) |
                          ((data >> i) & 1);
    }

    return e_success;
}
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int len = 0;

    while(file_extn[len] != '\0')
    {
        len++;
    }

    return encode_data_to_image(file_extn,
                                len,
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image);
}
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char size_data[4];

    size_data[0] = (file_size >> 24) & 0xFF;
    size_data[1] = (file_size >> 16) & 0xFF;
    size_data[2] = (file_size >> 8) & 0xFF;
    size_data[3] = file_size & 0xFF;

    return encode_data_to_image(size_data,
                                4,
                                encInfo->fptr_src_image,
                                encInfo->fptr_stego_image);
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    fseek(encInfo->fptr_secret, 0, SEEK_SET);

    while(fread(&ch, 1, 1, encInfo->fptr_secret) == 1)
    {
        encode_data_to_image(&ch,
                             1,
                             encInfo->fptr_src_image,
                             encInfo->fptr_stego_image);
    }

    return e_success;
}
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }

    return e_success;
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

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
