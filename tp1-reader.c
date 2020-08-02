#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FILE_NAME_DATA "Log.txt"
#define FILE_NAME_SIGN "Sign.txt"
#define FIFO_NAME "NamedFIFO"
#define FIFO_ALREADY_EXIST -1
#define BUFFER_SIZE 300
#define END_STRING '\0'
#define INPUT_TYPE_LENGTH 4

int main(void)
{
    char inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, f_name_fifo;
    FILE *f_data, *f_sign;
    char Input_Type[INPUT_TYPE_LENGTH];

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < FIFO_ALREADY_EXIST)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Create or open if exist, for add info in text file */
    f_data = fopen(FILE_NAME_DATA, "a+t");
    f_sign = fopen(FILE_NAME_SIGN, "a+t");

    /* Can't create files */
    if (f_data == NULL || f_sign == NULL)
    {
        printf("Error creating file \n");
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for writers...\n");
    if ((f_name_fifo = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", f_name_fifo);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        bytesRead = read(f_name_fifo, inputBuffer, BUFFER_SIZE);
        if (bytesRead > 0)
        {
            inputBuffer[bytesRead] = END_STRING;
            printf("reader: %s\n", inputBuffer);
            strncpy(Input_Type, inputBuffer, INPUT_TYPE_LENGTH);
            //printf("reader: %s\n", Input_Type);
            if (strncmp("DATA", Input_Type, INPUT_TYPE_LENGTH) == 0)
            {
                fprintf(f_data, "%s\n", inputBuffer);
            }
            if (strncmp("SIGN", Input_Type, INPUT_TYPE_LENGTH) == 0)
            {
                fprintf(f_sign, "%s\n", inputBuffer);
            }
        }
    } while (bytesRead > 0);

    /* Close files */
    fclose(f_data);
    fclose(f_sign);

    /* Close named fifo. */
    close(f_name_fifo);

    return 0;
}
