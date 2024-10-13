#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/signal.h>
#define LISTENQ 1024
#define MAXLINE 100
#define ANSWERLEN 6

void handler(int s) {
  // catch sigpipe
}

int main(int argc, char* argv[]) {
  signal(SIGPIPE, handler);
  // check command's arguments
  if(argc != 2) {
    printf("usage: ./serv <port>\n");
    exit(1);
  }
  // variables declaration
  int	i, maxi, maxfd, listenfd, connfd, sockfd;
	int	nready, client[FD_SETSIZE];
	ssize_t	n;
	fd_set rset, allset;
	char buf[MAXLINE];
	socklen_t	clilen;
	struct sockaddr_in	cliaddr, servaddr;
  char ipstr[INET_ADDRSTRLEN];
  char answer[] = "938132";


	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0,  sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ); // listen(int socket, int backlog);
	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++){
    client[i] = -1;			/* -1 indicates available entry */
  }
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

  // select
  for ( ; ; ) {
    rset = allset;		/* structure assignment */
    nready = select(maxfd+1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
      clilen = sizeof(cliaddr);
      connfd = accept(listenfd, (sockaddr*)&cliaddr, &clilen);
      printf("client connected from %s: %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, ipstr, sizeof(ipstr)), ntohs(cliaddr.sin_port));

      for (i = 0; i < FD_SETSIZE; i++) {
        if (client[i] < 0) {
          client[i] = connfd;	/* save descriptor */
          break;
        }
      }
      if (i == FD_SETSIZE) {
        printf("too many clients\n");
        exit(1);
      }

      FD_SET(connfd, &allset);	/* add new descriptor to set */
      if (connfd > maxfd) {
        maxfd = connfd;			/* for select */
      }
      if (i > maxi) {
        maxi = i;				/* max index in client[] array */
      }
      if (--nready <= 0) {
        continue;				/* no more readable descriptors */
      }
    }

    // check all clients for data 
    for (i = 0; i <= maxi; i++) {
      if ( (sockfd = client[i]) < 0) {
        continue;
      }
      if (FD_ISSET(sockfd, &rset)) {
        memset(buf, 0, MAXLINE);
        if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
          close(sockfd);
          FD_CLR(sockfd, &allset);
          client[i] = -1;
        } else {
          printf("%s", buf);
          for(int j = 0; j < ANSWERLEN; j++) {
            if(buf[j] != answer[j]) {
              // if the digit is not correct, return immediately
              write(sockfd, "wrong answer!\n", 14);
              break;
            } else {
              // if the digit is correct, sleep for awhile 
              // and check next digit
              usleep(10000);
            }
            if(j == ANSWERLEN - 1) {
              write(sockfd, "right answer!\n", 14);
            }
          }
        }

        // no more readable descriptors
        if (--nready <= 0)
          break;
      }
    }
	}

  return 0;
}