/* multi_proc_server.c - main 
 *
 * TCPechod.c 
 * */

#define	_USE_BSD
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
#include <fcntl.h>
#include "passiveTCP.c"
#include "passivesock.c"
#include "errexit.c"

#define	QLEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096

extern int	errno;

void	reaper(int);
int	transferFile(int sd);
int	errexit(const char *format, ...);
int	passiveTCP(const char *service, int qlen);

/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ECHO service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	char	*service = "28670";	/* service name or port number	*/
	struct	sockaddr_in fsin;	/* the address of a client	*/
	unsigned int	alen;		/* length of client's address	*/
	int	msock;			/* master server socket		*/
	int	ssock;			/* slave server socket		*/
	int 	clientNum=0;

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: ./server [port]\n");
	}

	msock = passiveTCP(service, QLEN);

	(void) signal(SIGCHLD, reaper);

	while (1) {
		alen = sizeof(fsin);
		printf ("\n---------------------------\n");
		printf ("Waiting for client...\n");
		printf ("---------------------------\n");
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			errexit("accept: %s\n", strerror(errno));
		}
		clientNum++;
		printf ("\n---------------------------\n");
		printf ("Client number %d connected\n", clientNum);
		printf ("---------------------------\n");

		switch (fork()) {
		case 0:		/* child */
			(void) close(msock);
			exit(transferFile(ssock));
		default:	/* parent */
			(void) close(ssock);
			break;
		case -1:
			errexit("fork: %s\n", strerror(errno));
		}
	}
}

/*------------------------------------------------------------------------
 * transferFile - transfer data
 *------------------------------------------------------------------------
 */
int
transferFile(int sd)
{
	char	buf[BUFSIZE];
	int	cc;
	int	n; //number of bytes sent
	int	fd; //file descriptor
	char 	*filename;

	// intialize buffer
	memset(buf, '\0', BUFSIZE);
	
	//read from socket (get filename)
 	cc = read (sd, buf, BUFSIZE);	
	
	if (cc < 0)
		errexit ("echo read: %s\n", strerror(errno));
	
	//store filename	
	filename = (char *) malloc (sizeof(buf));
	if (filename == NULL) exit(1);
	filename = buf;
	
	
	printf ("\n***************************\n");

	// open file
	if (fd = open(filename, O_RDONLY, 0)) {
		printf ("File opened.\n");
	} else {
		printf ("Error in reading file: %s\n", filename);
	}

	printf ("Reading file...\n");
	
	// Read from file and write to client socket 
	while ((n = read(fd, buf, BUFSIZE)) > 0) {
		write(sd, buf, n);
	}
	printf ("File written to client\n");
	printf ("***************************\n");

	close(fd);
	close(sd);	

	return 0;
}

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
void
reaper(int sig)
{
	int	status;

	while (wait3(&status, WNOHANG, (struct rusage *)0) >= 0)
		/* empty */;
}
