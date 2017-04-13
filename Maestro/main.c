#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "stream.h"
#define RECEIVER_BUFFER 10
#define TRANSIVER_BUFFER 30
char* reciverpath = "/dev/ttyACM2";
char* trasmiterpath = "/dev/ttyACM1";
char rbuf[RECEIVER_BUFFER] = {'0'};
char tbuffer[TRANSIVER_BUFFER];
/* Set up the control structure */
struct termios ttoptions, rtoptions;

int main(int argc, char *argv[])
{
    int rfd, tfd;
    /* Open the file descriptor in non-blocking mode */
    rfd = open(reciverpath, O_RDWR | O_NOCTTY);
    tfd = open(trasmiterpath, O_RDWR | O_NOCTTY);

    if (rfd < 0 || tfd < 0)
    {
        fprintf(stdout, "error can't open ttyACM");
        exit(EXIT_FAILURE);
    }


    /* Get currently set options for the tty */
    init_serial(rfd, &rtoptions);
    init_serial(tfd, &ttoptions);

    while (1)
    {
        int n = readline(tfd, tbuffer, TRANSIVER_BUFFER);
        readline(rfd, rbuf, RECEIVER_BUFFER);
        if (*tbuffer == '#')
            printline(tbuffer);
        else
        {
            if (*tbuffer == '!')
            {
                printline(rbuf);
                printline(tbuffer);
                tcsendbreak(rfd, 0);
                printf("debug");
                usleep(100000);
            }
        }

    }
    return 0;
}

