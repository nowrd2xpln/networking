/* TCPclient.c - main 
 *
 * TCPdaytime.c
 * */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "connectTCP.c"
#include "connectsock.c"
#include "errexit.c"
#include <fcntl.h>
#include <sys/file.h>

extern int	errno;

int	TCPclient(const char *host, const char *service);
int	errexit(const char *format, ...);
int	connectTCP(const char *host, const char *service);

#define	BUFSIZE	4096	

/*------------------------------------------------------------------------
 * main - TCP client for ECHO service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*host = "localhost";	/* host to use if none supplied	*/
	char	*service = "28670";	/* default service name		*/

	switch (argc) {
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
		fprintf(stderr, "usage: TCPclient [host [port]]\n");
		exit(1);
	}
	TCPclient(host, service);
	exit(0);
}

/*------------------------------------------------------------------------
 * TCPclient - request file from server  
 *------------------------------------------------------------------------
 */
int
TCPclient(const char *host, const char *service)
{
	char	buf[BUFSIZE];		/* buffer for one line of text	*/
	int	s, n;			/* socket descriptor, read count*/
	int	outchars, inchars;	/* characters sent and received	*/
	int	sqrt = 0;
        int	fd;

	s = connectTCP(host, service);

	printf("Enter filename> ");
	fgets(buf, sizeof(buf), stdin);	

	*strchr(buf, '\n') = '\0';
	buf[BUFSIZE] = '\0';	/* insure line null-terminated	*/
	outchars = strlen(buf);
	(void) write(s, buf, outchars);
	
	// create file
	fd = creat (buf, 0666);
	while (n = read(s, buf, BUFSIZE)){
		write (fd, buf, n);
	}
        close (fd);
	close (s);
}
