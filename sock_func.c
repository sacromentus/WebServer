#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sock_func.h"

void error (const char *msg)
{
  perror (msg);
  exit (1);
}

// Take this getServerSocketfd function as-is
// Choose your port number (portno), back comes an fd
int getServerSocketfd(int portno)
{
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error ("ERROR opening socket");
  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons (portno);
  if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof (struct sockaddr_in)) < 0)
    error ("ERROR on binding");
  if (listen (sockfd, 20)) // 20 is number of outstanding connections to allow
    error("Error on socket listen");
  return sockfd;
}

int getfiletype(char *path) {
	char temp[10];
	int i;

	for(i =  0; *(path +i) != '.' && *(path +i) != '\0'; i++)
				;
	int k;

	for(k = 0; (temp[k] = *(path +i) ) != '\0'; k++, i++)
				;
	temp[k] = '\0';

	if(strcmp(temp, ".jpg") == 0)
		return JPG;
	else if(strcmp(temp, ".gif") == 0)
		return GIF;
	else if(strcmp(temp, ".png") == 0)
		return PNG;

	return TEXT;
}

void init_header(Header *basic) {
	int i;
	
	basic->lines[0] = malloc(strlen("HTTP/1.1 200 OK\n")+1);
	basic->lines[1] = malloc(1000);
	basic->lines[2] = malloc(1000);
	basic->lines[3] = malloc(4);

	for(i = 0; i <= 3; i++)		//Check malloc()
		if(!basic->lines[i])
			error("malloc() failed!");

	strcpy(basic->lines[0], "HTTP/1.1 200 OK\n");
	strcpy(basic->lines[1], "Content-Length: ");
	strcpy(basic->lines[2], "Content-Type: ");
	strcpy(basic->lines[3], "\n");	
	basic->size = 0;
}
