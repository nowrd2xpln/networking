/* SERVER CODE */
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define QLEN    32 /* maximum connection queue length */
#define BUFSIZE 4096

extern int errno;

void reaper(int);
int TCPechod(int fd);
int errexit(const char *format, ...);
int passiveTCP(const char *service, int qlen);
/*------------------------------------------------------------------------
* main - Concurrent TCP server for ECHO service
*------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    char *service = "echo";  /* service name or port number */
    struct sockaddr_in fsin; /* the address of a client */
    unsigned int alen;       /* length of client's address */
    int msock;               /* master server socket */
    int ssock;               /* slave server socket */

    switch (argc)
    {
        case 1:
            break;
        case 2:
            service = argv[1];
            break;
        default:
            errexit("usage: TCPechod [port]\n");
    }

    msock = passiveTCP(service, QLEN);
    (void)signal(SIGCHLD, reaper);

    while (1)
    {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *)&fsin, &alen);

        if (ssock < 0)
        {
            if (errno == EINTR)
                continue;

            errexit("accept: %s\n", strerror(errno));
        }

        switch (fork())
        {
            case 0: /* child */
                (void)close(msock);
                exit(TCPechod(ssock));
            default: /* parent */
                (void)close(ssock);
                break;
            case -1:
                errexit("fork: %s\n", strerror(errno));
        }
    }
}

/*------------------------------------------------------------------------
* TCPechod - echo data until end of file
*------------------------------------------------------------------------*/
int TCPechod(int fd)
{
    char buf[BUFSIZ] = {0};
    char *ptr;
    int cc = 0;
    int originalNum = 0;
    int squareRoot = 0;
    int badNumberFlag = 0;
    float decimals = 0;

    while (memset(buf, '\0', BUFSIZ), cc = read(fd, buf, sizeof(buf)))
    {
        printf("RX %d-bytes:>%s<\n", cc, buf);

        if (cc < 0)
            errexit("echo read: %s\n", strerror(errno));

        for (ptr = buf; *ptr != '\0'; ptr++) // Check for non-digit character in input
        {
            if (*ptr < '0' || *ptr > '9')
                badNumberFlag = -1;
        }

        if (badNumberFlag == -1) // If input is invalid, send -1 to client
        {
            write(fd, &badNumberFlag, sizeof(int));
        }
        else // else send client square root of the input number
        {
            originalNum = atoi(buf);           // convert to integer
            squareRoot = sqrt(originalNum);    // Take sqrt

            if (originalNum % squareRoot == 0) // If input is a perfect square root
            {                                  // then send back to client
                printf("processed values: atoi(buf) = %d, sqrt(originalNum) = %d\n", originalNum, squareRoot);

                if (write(fd, &squareRoot, sizeof(int)) < 0)
                    errexit("echo write: %s\n", strerror(errno));
            }
            else // else the input is invalid and send back -1
            {
                badNumberFlag = -1;
                write(fd, &badNumberFlag, sizeof(int));
            }
        }

        badNumberFlag = 1;
    }
    return 0;
}

/*------------------------------------------------------------------------
* reaper - clean up zombie children
*------------------------------------------------------------------------
*/
void reaper(int sig)
{
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
        /* empty */;
}