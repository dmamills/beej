#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include "socket.h"

typedef struct raw_http_header {
  int length;
  char* content;
} raw_http_header;

typedef struct raw_http_body {
  int length;
  char* content;
} raw_http_body;


int parse_raw_header(char* content, int content_length, raw_http_header* header) {
  int headerEnd = -1;
  int curr = 0;
  while(1) {
     if(content[curr] == 0x0D && content[curr+1] == 0x0A && content[curr+2] == 0x0D && content[curr+3] == 0x0A) {
       headerEnd = curr;
       printf("found end of header at: %d\n\n", headerEnd);
       break;
    } else {
      curr++;
    }

    if(curr >= content_length) {
      break;
    }
  }

  header->length = headerEnd;
  header->content = (char*)malloc(headerEnd);
  memcpy(header->content, content, headerEnd);

  return headerEnd;
}

int parse_raw_body(char* content, int content_length, int header_length, raw_http_body* body) {
  body->length = content_length - (header_length + 4);
  body->content = (char*)malloc(body->length * sizeof(char));

  if(body->content == NULL) {
    printf("couldn't alloc memory");
    return -1;
  }

  memcpy(body->content, content + header_length + 4, body->length);
  return body->length;
}

int main(int argc, char **argv) {
  int sockfd; // file descriptor for socket
  int totalRead = 0; // number of bytes read in total
  char totalBuffer[CHUNK_SIZE * 4]; // big boy for holding all chunks
  char* message = "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: */*\r\nConnection: close\r\n\r\n"; // Our "GET" request

  if(argc == 1) {
    printf("Usage ./get <hostname>\n");
    return 1;
  }

  if(setup_socket(&sockfd, argv[1]) == 1) {
    printf("Couldnt init socket\n");
    return 1;
  }

  // send our GET request
  write(sockfd, message, strlen(message));

  bzero(totalBuffer, CHUNK_SIZE * 4);
  totalRead = read_till_end(totalBuffer, sockfd);

  raw_http_header raw_headers;
  if(parse_raw_header(totalBuffer, totalRead, &raw_headers) == -1) {
    printf("Couldnt parse headers.\n");
    return -1;
  }

  raw_http_body raw_body;
  if(parse_raw_body(totalBuffer, totalRead, raw_headers.length, &raw_body) == -1) {
    printf("Couldnt parse body.\n");
    return -1;
  }

  printf("Header:\n%s\n\n\n", raw_headers.content);
  printf("body:\n%s", raw_body.content);

  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);

  if(raw_body.content != NULL && raw_headers.content != NULL) {
    free(raw_body.content);
    free(raw_headers.content);
  }

  return 0;
}