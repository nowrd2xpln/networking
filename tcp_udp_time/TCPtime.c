#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "connectsock.c"
#include "connectTCP.c"
#include "errexit.c"

#define BUFSIZE 64
#define UNIXEPOCH 2208988800UL /* UNIX epoch, in UCT secs */

int connectTCP(const char *host, const char *service);
int errexit(const char *format, ...);
/*------------------------------------------------------------------------
* TCP client for TIME service that prints the resulting time 
*------------------------------------------------------------------------*/
int TCPtime(const char *host, const char *service)
/* * Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port, default to TIME */
{
    time_t now; /* 32-bit integer to hold time */
    int s, n;   /* socket descriptor, read count*/

    s = connectTCP(host, service);
    n = read(s, (char *)&now, sizeof(now));

    if (n < 0)
        errexit("read failed: %s\n", strerror(errno));
        
    now = ntohl((unsigned long)now); /* put in host order */
    now -= UNIXEPOCH;                /* convert UCT to UNIX epoch */
    printf("%s", ctime(&now));

    return (now);
}