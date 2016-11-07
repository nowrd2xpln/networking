/* multi_proc_server.c - main 
 *
 * TCPechod.c 
 * */

//#define	_USE_BSD
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
#include <pthread.h>

#define	QLEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096

extern int	errno;

void	reaper(int);
int	transferFile(int msock, pthread_mutex_t child_mutex);
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
	int	msock;			/* master server socket		*/
	int	numChildren=5;		/* # of children processes	*/
	int	count=0;
	pthread_mutex_t	child_mutex;
	pthread_mutexattr_t attr;

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
	(void) pthread_mutex_init(&child_mutex, &attr);

	//
	// returns 0 on success
	//
	if (! pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
		printf ("Success: Mutex is set as shared.\n");
	} else {
		printf ("Mutex not shared.\n");
	}
	
	while (1) {
		if ( count < numChildren ) {
			switch (fork()) {
			case 0:		/* child */
				//(void) close(msock);
				exit(transferFile(msock, child_mutex));
			default:	/* parent */
				//(void) close(ssock);
				count++;
				break;
			case -1:
				errexit("error fork: %s\n", strerror(errno));
			}
		}
	}
}

/*------------------------------------------------------------------------
 * transferFile - transfer data
 *------------------------------------------------------------------------
 */
int
transferFile(int msock, pthread_mutex_t child_mutex)
{
	char	buf[BUFSIZE];
	int	cc;
	int	n; //number of bytes sent
	int	fd; //file descriptor
	char 	*filename;
	struct	sockaddr_in fsin;	/* the address of a client	*/
	unsigned int	alen;		/* length of client's address	*/
	int	ssock;			/* slave server socket		*/

	printf ("\n---------------------------\n");
	printf ("Server forked...\n");
	printf ("---------------------------\n");
  while (1) {
	alen = sizeof(fsin);

	pthread_mutex_lock(&child_mutex);
	ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
	if (ssock < 0) {
		if (errno == EINTR)
		errexit("accept: %s\n", strerror(errno));
	}
	pthread_mutex_unlock(&child_mutex);

	printf ("\n======Mutex Unlocked======\n");

	printf ("\n---------------------------\n");
	printf ("Client connected\n");
	printf ("---------------------------\n");

	// intialize buffer
	memset(buf, '\0', BUFSIZE);
	
	//read from socket (get filename)
 	cc = read (ssock, buf, BUFSIZE);	
	
	if (cc < 0)
		errexit ("echo read: %s\n", strerror(errno));
	
	//store filename	
	filename = (char *) malloc (sizeof(buf));
	if (filename == NULL) exit(1);
	filename = buf;
	
	printf ("\n***************************\n");
	
	if (filename == "quit") {
		close(ssock);
		return(0);
	} else {
	
		// open file
		if (fd = open(filename, O_RDONLY, 0)) {
			printf ("File opened.\n");
		} else {
			printf ("Error in reading file: %s\n", filename);
		}

		printf ("Reading file...\n");
	
		// Read from file and write to client socket 
		while ((n = read(fd, buf, BUFSIZE)) > 0) {
			write(ssock, buf, n);
		}
		printf ("File written to client\n");
		printf ("***************************\n");

		close(fd);
		close(ssock);	
	}
  }
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
