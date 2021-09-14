#ifndef SOCKET_HEADER
#define SOCKET_HEADER
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef PORT
#define PORT 80
#endif

#ifndef CHUNKS_SIZE
#define CHUNK_SIZE 1024
#endif

/**
 * Prepares and connects a socket to a given hostname
 * @param int* socket file descriptor
 * @param char* hostname, the string url
 */
int setup_socket(int* fd, char* hostname) {
  struct hostent *hp; // struct holding host name information
  struct sockaddr_in addr; // struct for holding socket information like ip/port
  if(strcmp(hostname, "http://") > 0) {
    hp = gethostbyname(hostname + strlen("http://"));
  } else {
    hp = gethostbyname(hostname);
  }

  if(hp == NULL) {
    printf("Unable to get hostname\n");
    return 1;
  }

  //copy ip from hp struct to  sockaddr_in struct
  bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
  addr.sin_port = htons(PORT);
  addr.sin_family = AF_INET;

  // create socket
  int on = 1;
  *fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  setsockopt(*fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on, sizeof(int));

  if(*fd == -1) {
    printf("Couldnt init socket\n");
    return 1;
  }

  //setup socket to connect
  if(connect(*fd, (struct sockaddr*)&addr, sizeof (struct sockaddr_in)) == -1) {
    printf("unable to connect\n");
    return 1;
  }

  return 0;
}

/**
 * Reads a socket until it ends
 * @param char* buffer to write each chunk to
 * @param int socket file descriptor to read from
 * @returns the total number of bytes read
 */
int read_till_end(char* endBuffer, int sockfd) {
  char buffer[CHUNK_SIZE]; // buffer for current chunk
  int nread = 0;
  int totalRead = 0;

  fcntl(sockfd, F_SETFL, O_NONBLOCK); //set socket to non blocking
  while(1) {
    bzero(buffer, CHUNK_SIZE);
    if((nread = recv(sockfd, buffer, CHUNK_SIZE, MSG_DONTWAIT)) < 0) {
      usleep(1000000);
    } else if(nread == 0) {

      break;
    } else {
      memcpy(endBuffer + totalRead, buffer, nread);
      totalRead += nread;
    }
  }

  return totalRead;
}

#endif