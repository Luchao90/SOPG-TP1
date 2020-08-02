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
#define BUFFER_SIZE 50
#define MSG_SIGUSER_1 "SIGN:1"
#define MSG_SIGUSER_2 "SIGN:2"
#define MSG_CRTL_C "CONTROL C"

void SigUser_1(int sig);
void SigUser_2(int sig);

int32_t returnCode, f_name_fifo;

int main(void)
{
    char outputBuffer[BUFFER_SIZE + sizeof("DATA:")];
    char KeyboardBuffer[BUFFER_SIZE];

    struct sigaction User1;
    struct sigaction User2;

    User1.sa_handler = SigUser_1;
    User1.sa_flags = SA_RESTART;
    sigemptyset(&User1.sa_mask);

    User2.sa_handler = SigUser_2;
    User2.sa_flags = SA_RESTART;
    sigemptyset(&User2.sa_mask);

    /* Map handlers for user signals */
    sigaction(SIGUSR1, &User1, NULL);
    sigaction(SIGUSR2, &User2, NULL);

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
    printf("got a reader--type some stuff\n\n");

    /* Loop forever */
    while (1)
    {
        /* Get some text from console */
        if (NULL != fgets(KeyboardBuffer, BUFFER_SIZE, stdin))
        {
            sprintf(outputBuffer, "DATA:%s", KeyboardBuffer);

            /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
            if ((write(f_name_fifo, outputBuffer, strlen(outputBuffer) - 1)) == -1)
            {
                perror("write");
            }
        }
    }

    /* Close named fifo. */
    close(f_name_fifo);
    return 0;
}

void SigUser_1(int sig)
{
    write(f_name_fifo, MSG_SIGUSER_1, sizeof(MSG_SIGUSER_1));
}

void SigUser_2(int sig)
{
    write(f_name_fifo, MSG_SIGUSER_2, sizeof(MSG_SIGUSER_2));
}
