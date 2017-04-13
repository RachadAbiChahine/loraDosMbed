
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "stream.h"



void init_serial(int fd, struct termios* toptions)
{
    tcgetattr(fd, toptions);
    /* Set custom options */

    /* 9600 baud */
    cfsetispeed(toptions, B38400);
    cfsetospeed(toptions, B38400);
    /* 8 bits, no parity, no stop bits */
    toptions->c_cflag &= ~PARENB;
    toptions->c_cflag &= ~CSTOPB;
    toptions->c_cflag &= ~CSIZE;
    toptions->c_cflag |= CS8;
    /* no hardware flow control */
    toptions->c_cflag &= ~CRTSCTS;
    /* enable receiver, ignore status lines */
    toptions->c_cflag |= CREAD | CLOCAL;
    /* disable input/output flow control, disable restart chars */
    toptions->c_iflag &= ~(IXON | IXOFF | IXANY);
    /* disable canonical input, disable echo,
    disable visually erase chars,
    disable terminal-generated signals */
    toptions->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    /* disable output processing */
    toptions->c_oflag &= ~OPOST;

    /* wait for 12 characters to come in before read returns */
    /* WARNING! THIS CAUSES THE read() TO BLOCK UNTIL ALL */
    /* CHARACTERS HAVE COME IN! */
    toptions->c_cc[VMIN] = 12;
    /* no minimum time to wait before read returns */
    toptions->c_cc[VTIME] = 0;
    /* commit the options */
    tcsetattr(fd, TCSANOW, toptions);
    /* Wait for the Arduino to reset */
    /* Flush anything already in the serial buffer */
    tcflush(fd, TCIFLUSH);
    /* read up to 128 bytes from the fd */
}

int readline(int fd, char*buffer, int buffersize)
{
    int count = 0;
    int res = 0;
    char c[1];
    while (((count = read(fd, c, 1)) != -1))
    {
        if ((c[0] == '\n') || (c[0] == EOF)) break;
        buffer[res++] = *c;

    }
    if (*c == '\n') buffer[++res] = '\n';
    else
    {
        fprintf(stdout, "EOF");

        exit(EXIT_FAILURE);
    }
    return res + 1;
}

void printline(char* buffer)
{
    uint16_t i = 0;
    while (*(buffer + i) != '\n')
    {
        printf("%c", *(buffer + i));
        ++i;
    }
    printf("\n");
}