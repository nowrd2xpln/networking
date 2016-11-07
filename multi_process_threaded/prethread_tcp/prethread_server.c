/*
 * multi_thread_server.c - main
 *
 * TCPmtechod.c  */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include "errexit.c"
#include "passiveTCP.c"
#include "passivesock.c"

#define	QLEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096

pthread_mutex_t child_mutex;
pthread_mutexattr_t attr;

int	fileTransfer(void * msock);
int	errexit(const char *format, ...);
int	passiveTCP(const char *service, int qlen);

/*------------------------------------------------------------------------
 * main - Concurrent TCP server file transfer (using threads) 
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	pthread_t	th;
	pthread_attr_t	ta;
	pthread_mutexattr_t attr;
	int 	count=0;
	int 	numChildren=5;
	int 	msock;
	char *service = "28670";

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

	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);
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
		if ( count < numChildren) {
			if (pthread_create(&th, &ta, (void * (*)(void *))fileTransfer,
		    	(void *)msock) < 0)
				errexit("pthread_create: %s\n", strerror(errno));
			count++;
		}
	}
}

/*------------------------------------------------------------------------
 * fileTransfer - transfer data  
 *------------------------------------------------------------------------
 */
int
fileTransfer(void *msock )
{
	char	buf[BUFSIZE];
	int	cc;
	int 	n; //number of bytes sent
	int 	fd; //file descriptor
	char 	*filename;
	struct	sockaddr_in fsin;	/* the address of a client	*/
	unsigned int	alen;		/* length of client's address	*/
	int	ssock;			/* slave server socket		*/

	printf ("\n------------------------\n");
	printf ("Created thread...\n");
	printf ("------------------------\n");
  while (1) {	
	alen = sizeof(fsin);
	pthread_mutex_lock(&child_mutex);
	ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
	if (ssock < 0) {
		if (errno == EINTR)
		errexit("accept: %s\n", strerror(errno));
	}
	printf ("\n------------------------\n");
	printf ("Client connected!\n");
	printf ("------------------------\n");
	pthread_mutex_unlock(&child_mutex);

	//initialize buffer
        memset(buf, '\0', BUFSIZE);

	//read from socket (get filename)
        cc = read (ssock, buf, sizeof(buf));
        if (cc < 0)
            errexit("echo read: %s\n", strerror(errno));
	
	//store filename
        filename = (char *) malloc (sizeof(buf));
        if (filename==NULL) exit (1);
        filename = buf;
	
	if (filename == "quit") {
		close (ssock);
		return 0;
	} else {
		printf ("\n**********************\n");
	        printf ("Filename: %s\n", filename);
	
		//open file
        	if (fd = open(filename, O_RDONLY, 0)){
            	printf ("File opened\n");
        	} else {
            	printf ("Error in reading file: %s\n", filename);
        	}

       		printf ("Reading file...\n");

		// Read from file and write to client socket
        	while ((n = read(fd, buf, BUFSIZE)) > 0) {
            	write (ssock, buf, n);
        	}
        	printf ("File written to client.\n");
		printf ("\n**********************\n");

        	close (fd);
		close(ssock);
	}
  } //while
}

