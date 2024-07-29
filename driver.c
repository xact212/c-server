#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_PORT 8080
#define TRANS_BUF_SIZE 256


int main (int argc, char** argv) {
  //setup main network socket
  int acceptSock = socket(AF_INET, SOCK_STREAM, 0);
  if (acceptSock < 0) {
    perror("could not create network socket");
    exit(1);
  }
  puts("successfully created main socket");
  //attempt to obtain user specified port. 
  //on failure simply use default port
  int acceptPort = DEFAULT_PORT;
//  if (argc > 0) {
//    acceptPort = atoi(argv[1]); 
//  }
  //bind socket to interface address
  const struct sockaddr_in acceptSockAddr = {
    AF_INET,
    htons(acceptPort),
    INADDR_ANY
  };
  if (bind(acceptSock, (struct sockaddr*)&acceptSockAddr, sizeof(acceptSockAddr)) < 0) {
    perror("could not bind socket to address"); 
    exit(1);
  }
  puts("successfully bound main socket to address"); 
  //listen for incoming connections on the accept socket
  listen(acceptSock, 8);
  printf("main socket listening on port %i\n", acceptPort);
  
  //initialzie transmission buffers
  char incomingBuf[TRANS_BUF_SIZE]; //incoming
  memset(incomingBuf, '\0', TRANS_BUF_SIZE);
  char outgoingBuf[TRANS_BUF_SIZE]; //outgoing
  memset(outgoingBuf, '\0', TRANS_BUF_SIZE);
  
  //accept to create a new socket to handle the client connection
  int sockAddrSize = sizeof(acceptSockAddr);
  int newSocket = accept(acceptSock, 0, &sockAddrSize);
  if (newSocket < 0) {
    perror("error creating new socket to handle client request");
    exit(1);
  }
  printf("connection established with client on new socket %i\n", newSocket);
  //recieve request from client
  int bytesRecieved = recv(newSocket, (void*)incomingBuf, 256, 0);
  if (bytesRecieved < 0) {
    perror("error recieving bytes from the client");
    exit(1);
  }
  printf("successfully recieved %i bytes from the client\n", bytesRecieved);
  printf("message contents: %s\n", incomingBuf);
  //respond to client
  puts("sending http response to client"); 
  FILE* fileToSendHand = fopen("index.html", "r");
  if (!fileToSendHand) {
    perror("retreival of requested file to determine length failed");
    exit(1);
  }
  //find length of file you are sending
  int fileLen = 0; 
  while (getc(fileToSendHand) != EOF) {
    fileLen++;
  }
  //use open to get the file descriptor
  fclose(fileToSendHand);
  int fileToSendFD = open("index.html", O_RDONLY);
  if (fileToSendFD < 0) {
    perror("retreival of requested file to send to client failed");
    exit(1);
  }
  sendfile(newSocket, fileToSendFD, 0, fileLen);
  //cleanup any open file descriptors
  close(fileToSendFD);
  close(newSocket);
  close(acceptSock);
}
