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

#include "errexit.c"
#include "passiveTCP.c"
#include "passivesock.c"

#define	QLEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096

int	fileTransfer(int sd);
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

	(void) pthread_attr_init(&ta);
	(void) pthread_attr_setdetachstate(&ta, PTHREAD_CREATE_DETACHED);

	while (1) {
		alen = sizeof(fsin);
		printf ("\n------------------------\n");
		printf ("Waiting for client...\n");
		printf ("------------------------\n");
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
		if (ssock < 0) {
			if (errno == EINTR)
				continue;
			errexit("accept: %s\n", strerror(errno));
		}
		clientNum++;
		printf ("\n------------------------\n");
		printf ("Client number %d connected!\n", clientNum);
		printf ("------------------------\n");
		if (pthread_create(&th, &ta, (void * (*)(void *))fileTransfer,
		    (void *)ssock) < 0)
			errexit("pthread_create: %s\n", strerror(errno));
	}
}

/*------------------------------------------------------------------------
 * fileTransfer - transfer data  
 *------------------------------------------------------------------------
 */
int
fileTransfer(int sd)
{
	char	buf[BUFSIZE];
	int	cc;
	int 	n; //number of bytes sent
	int 	fd; //file descriptor
	char 	*filename;
	
	//initialize buffer
        memset(buf, '\0', BUFSIZE);

	//read from socket (get filename)
        cc = read (sd, buf, sizeof(buf));
        if (cc < 0)
            errexit("echo read: %s\n", strerror(errno));
	
	//store filename
        filename = (char *) malloc (sizeof(buf));
        if (filename==NULL) exit (1);
        filename = buf;

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
            write (sd, buf, n);
        }
        printf ("File written to client.\n");
	printf ("\n**********************\n");

        close (fd);
	close(sd);
	return 0;
}

