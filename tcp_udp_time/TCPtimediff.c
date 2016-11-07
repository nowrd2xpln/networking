#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "TCPtime.c"

#define BUFSIZE 64 #define UNIXEPOCH 2208988800UL /* UNIX epoch, in UCT secs */

/*------------------------------------------------------------------------
* TCPtimediff main - TCP client for TIME service that prints the resulting time
*----------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    char *hostA;            /* First TIME host */
    char *hostB;            /* Second TIME host */
    char *service = "time"; /* default service to TIME */
    int serverA;            /* TIME representation of first server */
    int serverB;            /* TIME representation of second server */

    switch (argc)
    {
    case 3:
        hostA = argv[1];    // Get first hostname from cmd line
        hostB = argv[2];    // Get second hostname from cmd line
        break;
    default:
        fprintf(stderr, "Error. Please enter two valid TIME hosts.\n");
        exit(1);
    }
    // Run TCPtime to print the time from the first host
    // Store the numerical time in the serverA variable
    printf("Server A's time is: ");
    serverA = TCPtime(hostA, service);

    // Sleep the program for 1 second to show the time difference sleep(1);
    // Run TCPtime to print the time from the second host
    // Store the numerical time in the serverB variable
    printf("Slept for 1 second...\n");
    printf("Server B's time is: ");
    serverB = TCPtime(hostB, service);

    // Print the time difference
    printf("The time difference is: %d\n", (serverB - serverA));
    
    exit(0);
}