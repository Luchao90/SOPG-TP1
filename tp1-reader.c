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

int main(void)
{
    uint8_t inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, f_name_fifo;
    FILE *f_data, *f_sign;
    char character;

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
        }
    } while (bytesRead > 0);

    /* Close files */
    fclose(f_data);
    fclose(f_sign);

    return 0;
}
