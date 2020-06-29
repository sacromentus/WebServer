#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "sock_func.h"
#include <signal.h>

#include <sys/stat.h>
#include <fcntl.h>


#define PORT 	8135
#define BUFLEN 	9999
#define SIZE 	10000
#define TRUE	1
#define	FALSE	0

int serverSocket;

void communicate(int);
void transmit(char *, int);
int locate(char *, char *, Header *);	//Dissects client's request and finds file; returns 1 if found, 0 not found
int getfiletype(char *);

void shut(int k) {		//Function is called to close the socket when the program receives any SIGNAL interrupt
	close(serverSocket);
	exit(4);
}	
	
int main() {
	int PID;	
	serverSocket   = getServerSocketfd(PORT);
	signal(SIGINT, shut);
	
	while(1) {
		struct sockaddr_in cli_addr;
		socklen_t clilen = sizeof(struct sockaddr_in);
		int clientSocketfd = accept(serverSocket, (struct sockaddr *) &cli_addr, &clilen); //wait for client to connect
		if(clientSocketfd < 0)
			fprintf(stderr, "ERROR on accept");
		communicate(clientSocketfd);

	}
	close(serverSocket);
	return 0;
}	

void communicate(int clientSocketfd) {
	char buffer[BUFLEN];
	char from[100];
	int n;
	int found;	
	Header *basic = malloc(sizeof(Header));
	
	if(!basic)
		error("Error: malloc() failed!");

	n = read (clientSocketfd, buffer, BUFLEN - 1);
	buffer[n] = '\0';
	found = locate(buffer, from, basic);

	if(found) {	//We only send header first if the file is found; apparently the NOT FOUND page conatains its own	
		int k, length;
		for(k = 0;k <= 3; k++) {
			length = write(clientSocketfd, basic->lines[k], strlen(basic->lines[k]));
		
			if(length < 0)
				fprintf(stderr, "Error sending header");
		}	
	}

	transmit(from, clientSocketfd);
	free(basic);
	close(clientSocketfd);
}

int locate(char *buffer, char *path, Header *basic) {
	char temp[100];
	char filename[100];	//this will hold only the name of the file so we can determine its type
	int file_found = TRUE;

	if(strncmp(buffer, "GET", 3) != 0)
		exit(4);	// Not a valid HTTP Request if it doesn't begin with 'GET'	

	int i = 0;

	/*	Retrieve filename and have stat() populate buf with information	*/
		
	while( (temp[i] = *(buffer + i + 4))  != '\0' && temp[i] != ' '  && temp[i] != '\n' && temp[i] != '\r' && temp[i] != '\t')	//Extract the file requested
				i++;
	temp[i] = '\0';

	if(strncmp(temp, "/\0", 2) == 0) {	//Handle Default Page
		strcpy(path, "./pages/default.html");
		strcpy(filename, "default.html");
	}
	
	else  {					//Otherwise fill pathname into path
		temp[i] = '\0';
		strcpy(path, "./pages");
		strcat(path, temp);
		strcpy(filename, temp);
	}
	
	struct stat *buf = malloc(sizeof(struct stat));

	if(!buf)
		error("malloc() failed!");


	if( stat(path, buf) == -1) {	// Call stat to retrieve file information; -1 means file not found
		strcpy(path, "./pages/NOTFOUND\0");
		stat(path, buf);
		strcpy(filename, "NOTFOUND\0");
		file_found = FALSE;
	}

	/*	We have the file, now we fill in the header	*/
	int filetype = getfiletype(filename);	
	char size[20];
	init_header(basic);
	snprintf(size, 18, "%d\n", buf->st_size);	//transfer size stat into a char array
	strcat(basic->lines[1], size);		//Fill in the Content-Length header

	switch(filetype) {			// Fill in Content-Type header
		case JPG:
			strcat(basic->lines[2], "image/jpg\n");	
			break;
		case GIF:
			strcat(basic->lines[2], "image/gif\n");
			break;
		case PNG:
			strcat(basic->lines[2], "image/png\n");
			break;
		case TEXT:
			strcat(basic->lines[2], "text/html; charset=UTF-8\n");
			break;
		default:
			break;
	}
	free(buf);
	return file_found;
}
	
	

void transmit(char *fromName, int fd) {
	int fromFile = open(fromName, O_RDONLY);
	if (fromFile < 0) {  // ALWAYS check result of open
		perror("Could not open input file");
		exit(4);
	}

	char *buffer = malloc(SIZE);
	
	if (!buffer) {
		fprintf(stderr, "Unable to allocate buffer memory\n");
		exit(4);
	}

	ssize_t bytesRead;
	while ( (bytesRead = read(fromFile, buffer, SIZE))>0) {
		//printf("bytes read: %d\n", bytesRead);
		ssize_t bytesWritten = write(fd, buffer, bytesRead);
		if (bytesWritten == -1) {
			perror("error writing file");
			exit(4);
		}
		if (bytesRead != bytesWritten)
			fprintf(stderr, "Warning: %d bytes read, but only %d bytes written",
			                (int)bytesRead, (int)bytesWritten);
			// should we exit here?
	}
	if (bytesRead == -1) {
		perror("error reading file");
		exit(4);
	}
	
	close(fromFile); // always close files when finished with them
}

