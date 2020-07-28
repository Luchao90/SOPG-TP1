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

#define FIFO_NAME "NamedFIFO"
#define FIFO_ALREADY_EXIST -1
#define BUFFER_SIZE 300
#define MSG_SIGUSER_1 "SIGN:1\n"
#define MSG_SIGUSER_2 "SIGN:2\n"
#define MSG_CRTL_C "CONTROL C\n"

void SigUser_1(int sig);
void SigUser_2(int sig);
void Ctrl_C(int sig);

int32_t returnCode, f_name_fifo;

int main(void)
{
    char outputBuffer[BUFFER_SIZE];
    uint32_t bytesWrote;
    signal(SIGUSR1, SigUser_1);
    signal(SIGUSR2, SigUser_2);
    signal(SIGINT, Ctrl_C);
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < FIFO_ALREADY_EXIST)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for readers...\n");
    if ((f_name_fifo = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", f_name_fifo);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a reader--type some stuff\n");

    /* Loop forever */
    while (1)
    {
        /* Get some text from console */
        fgets(outputBuffer, BUFFER_SIZE, stdin);

        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
        if ((bytesWrote = write(f_name_fifo, outputBuffer, strlen(outputBuffer) - 1)) == -1)
        {
            perror("write");
        }
        else
        {
            printf("writer: wrote %d bytes\n", bytesWrote);
        }
    }
    return 0;
}

void Ctrl_C(int sig)
{
    write(f_name_fifo, MSG_CRTL_C, sizeof(MSG_CRTL_C));
}

void SigUser_1(int sig)
{
    write(f_name_fifo, MSG_SIGUSER_1, sizeof(MSG_SIGUSER_1));
}

void SigUser_2(int sig)
{
    write(f_name_fifo, MSG_SIGUSER_2, sizeof(MSG_SIGUSER_2));
}
