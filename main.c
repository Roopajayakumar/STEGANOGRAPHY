#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>
// encode ./a.out -e source.bmp secret.txt output.bmp
//  ./a.out -d output.bmp secret.txt
//argc====>count
//argc========>value
int main(int argc,char *argv[])
{
    EncodeInfo encInfo;
    if(argc < 2)
    {
        printf("Error: Please pass -e or -d\n");
        return 0;
    }
    OperationType res = check_operation_type(argv);

    if(res == e_encode)
    {
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Arguments validated successfully\n");

            if(open_files(&encInfo) == e_success)
            {
                printf("Files opened successfully\n");

                if(check_capacity(&encInfo) == e_success)
                {
                    printf("Capacity is sufficient\n");
                    if(do_encoding(&encInfo) == e_success)
                    {
                        printf("Encoding completed successfully\n");
                    }
                    else
                    {
                        printf("Encoding failed\n");
                    }
                }
                else
                {
                    printf("Capacity is not sufficient\n");
                }
            }
            else
            {
                printf("Failed to open files\n");
            }
        }
        else
        {
            printf("Invalid arguments\n");
        }
    }
    else if(res == e_decode)
    {
        DecodeInfo decInfo;

        if(read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Decode arguments validated successfully\n");

            if(open_decode_files(&decInfo) == e_success)
            {
                printf("Decode files opened successfully\n");

                if(do_decoding(&decInfo) == e_success)
                {
                    printf("Decoding completed successfully\n");
                }
                else
                {
                    printf("Decoding failed\n");
                }
            }
            else
            {
                printf("Failed to open decode files\n");
            }
        }
        else
        {
            printf("Invalid decode arguments\n");
        }
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1],"-e")==0)
    {
        printf("encode selected\n");
        return  e_encode;
    }
    else if(strcmp(argv[1],"-d")==0)
    {
        printf("decode selected");
        return  e_decode;
    }
    else
    {
        printf("unsupported");
        return e_unsupported;
    }
}