#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "connectUDP.c"
#include "connectsock.c"
#include "errexit.c"
#include <errno.h>

//define max buffer size and message to send to server
#define LINELEN 128
#define MSG "Give me the daytime\n"

// extern int errno;
int UDPdaytime(const char *host, const char *service);
int errexit(const char *format, ...);
int connectUDP(const char *host, const char *service);

/*------------------------------------------------------------------------
* main - UDP client for DAYTIME service
* ------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char *host = "localhost";  /* host to use if none supplied */
    char *service = "daytime"; /* default service port */
    
    /* only accept 1 or 2 arguments host and/or service */
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
        fprintf(stderr, "usage: UDPdaytime [host [port]]\n");
        exit(1);
    }
    // call function to handle DAYTIME service
    UDPdaytime(host, service);
    exit(0);
}

/*------------------------------------------------------------------------
* UDPdaytime - invoke Daytime on specified host and print results
*------------------------------------------------------------------------*/
UDPdaytime(const char *host, const char *service)
{
    char buf[LINELEN + 1]; /* buffer for one line of text */
    int s, n;              /* socket, read count */
    // connect to UDP server
    s = connectUDP(host, service);
    // send message to signal server
    (void)write(s, MSG, strlen(MSG));
    // for udp protocol, only need to read once
    n = read(s, buf, sizeof(buf));
    // display error message if no message received
    if (n < 0)
    {
        printf("read failed\n");
        exit(1);
    }
    // display daytime information
    printf("%s\n", &buf);
}