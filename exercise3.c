#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

void capitalizeBuffer(char *buffer) {
  do {
      *buffer = toupper((unsigned char) *buffer);
  } while (*buffer++);
}

int main(int argc, char *argv[]) {
     int sockfd, newsockfd, n;
     uint clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(8888);
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");
     listen(sockfd, 5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) error("ERROR on accept");
     bzero(buffer,256);
     n = read(newsockfd, buffer, 255);
     if (n < 0) error("ERROR reading from socket");
     capitalizeBuffer(buffer);
     printf("Here is the message: %s\n", buffer);
     n = write(newsockfd, buffer, n);
     if (n < 0) error("ERROR writing to socket");
     return 0; 
}