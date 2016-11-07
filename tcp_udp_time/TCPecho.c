#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define LINELEN 128

extern int errno;

int TCPechoSQRT(const char *host, const char *service);
int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);


/*------------------------------------------------------------------------
* main - TCP client for ECHOing the square root of a number
*------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char *host = "localhost"; /* host to use if none supplied */
    char *service = "echo";   /* default service name */

    switch (argc)
    {
        case 1:
            host = "localhost";
            break;
        case 3:
            service = argv[2];
            /* FALL THROUGH */
        case 2:
            host = argv[1];
            break;
        default:
            fprintf(stderr, "usage: echosqrt [host [port]]\n");
            exit(1);
    }

    TCPechoSQRT(host, service);
    exit(0);
}

/*------------------------------------------------------------------------
* TCPechoSQRT - send input to ECHO service on specified host and print
* the integer square root of a number.
*------------------------------------------------------------------------*/
int TCPechoSQRT(const char *host, const char *service)
{
    char buf[LINELEN + 1]; /* buffer for one line of text */
    int s = 0;             /* socket descriptor*/
    int squareRoot = 0;    /* Holds square root value */

    s = connectTCP(host, service);

    // Prompt user until "quit"
    while (printf("Enter number> "),
           fgets(buf, sizeof(buf), stdin),
           strncmp(buf, "quit", 4))
    {
        *strchr(buf, '\n') = '\0';          // chomp off '\n' ...not that important
                                            // but it makes it easier for the server.
        write(s, buf, strlen(buf));         // Write to socket
        read(s, &squareRoot, sizeof(int));  // Read from socket

        if (squareRoot < 0)
            printf("Error! Please enter valid number.\n\n");
        else
            printf("RX'ed: squareRoot = %d\n", squareRoot);
    }
}