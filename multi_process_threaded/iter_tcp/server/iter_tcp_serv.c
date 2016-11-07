/* iter_tcp_serv.c  - main
 *
 * TCPdaytimed.c
 *
 * */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#include "passiveTCP.c"
#include "passivesock.c"
#include "errexit.c"

#define BUFSIZE	4096

extern int	errno;
int		errexit(const char *format, ...);
void		transferFile(int sd);
int		passiveTCP(const char *service, int qlen);

#define QLEN	32

/*------------------------------------------------------------------------
 * main - Iterative TCP server 
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct	sockaddr_in fsin;	/* the from address of a client	*/
	char	*service = "28670";	/* service name or port number	*/
	int	msock, ssock;		/* master & slave sockets	*/
	unsigned int	alen;		/* from-address length		*/
	int 	connections = 0;

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: <prog_name> [port]\n");
	}

	msock = passiveTCP(service, QLEN);

	while (1) {
		alen = sizeof(fsin);
		printf("\n---------------------------\n");
		printf ("Waiting for client...\n\n");
		ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
		if (ssock < 0)
			errexit("accept failed: %s\n", strerror(errno));
		connections++;
		printf ("Connection number %d\n", connections);
		transferFile(ssock);
		(void) close(ssock);
	}
}

/*------------------------------------------------------------------------
 * Transfer File 
 *------------------------------------------------------------------------
 */
void
transferFile(int sd)
{
        char	buf[BUFSIZE];
        int 	cc;
	int 	n; //number of bytes sent
        char	*filename;
        int	fd; //file descriptor	

        memset(buf, '\0', BUFSIZE);
        cc = read (sd, buf, sizeof(buf));
        if (cc < 0) 
   	    errexit("echo read: %s\n", strerror(errno));

	filename = (char *) malloc (sizeof(buf));        
        if (filename==NULL) exit (1);

        filename = buf;

        printf ("Filename: %s\n", filename); 
        
        if (fd = open(filename, O_RDONLY, 0)){
	    printf ("File opened\n");
	} else {
	    printf ("Error in reading file: %s\n", filename);
	}

	printf ("Reading file...\n");
	
	while ((n = read(fd, buf, BUFSIZE)) > 0) {
	    write (sd, buf, n);
	}
	printf ("File written to client.\n");
        close (fd);
}

