#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/time.h>
#define maxline 100
#define ANSWERLEN 6


double tv2s(struct timeval *ptv) {
	return 1.0*(ptv->tv_sec) + 0.000001*(ptv->tv_usec);
}

void str_cli(int sockfd) {
  char sendline[maxline], recvline[maxline];
  char number[maxline];
  char atoi[2];
  int numlen=0;
  struct timeval start, end;
  double totaltime, averagetime;
  double timearray[10];
  int max;
  memset(number, 0, maxline);
  memset(timearray, 0, sizeof(timearray));

  while(numlen < ANSWERLEN) {
    for(int i=0; i < 10; i++) {
      memset(sendline, 0, maxline);
      snprintf(sendline, numlen+3, "%s%d\n", number, i);

      totaltime = 0;
      for(int j=0; j < 1; j++) { // accumulate spending time so that we can easier distinguish the number
        memset(recvline, 0, maxline);
        gettimeofday(&start, NULL); // get starting time
        write(sockfd, sendline, strlen(sendline));
        read(sockfd, recvline, maxline);
        gettimeofday(&end, NULL); // get ending time
        totaltime += tv2s(&end) - tv2s(&start);
      }
      timearray[i] = totaltime;
      // print result
      printf("%stime spent: %f\n", sendline, timearray[i]);
      if(strncmp(recvline, "right answer!\n", 14) == 0) {
        printf("The answer is %s%d\n", number, i);
        return;
      }
    }
    max = 0;
    for(int i=0; i<10; i++) {
      if(timearray[i] > timearray[max]) {
        max = i;
      }
    }
    snprintf(atoi, 2, "%d\n", max);
    strncpy(number+numlen, atoi, 1);
    printf("------------------------------number=%s\n", number);
    numlen++;
  }  
}

int main(int argc, char* argv[]) {
  int sockfd;
  struct sockaddr_in servaddr;
  int port = 10001;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
  connect(sockfd, (sockaddr*) &servaddr, sizeof(servaddr));
  str_cli(sockfd);
  return 0;
}