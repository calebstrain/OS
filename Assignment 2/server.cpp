#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/wait.h>
#include <netinet/in.h> 
#include <string>
#include <iostream>
#include <cstring>

using namespace std;

void createCode(int); /* function prototype */
void error(string msg)
{
	perror(msg.c_str());
	exit(1);
}

void fireman(int)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		std::cout << "A child process ended" << std::endl;
}

int main(int argc, char* argv[])
{
	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;

	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char*)& serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr*) & serv_addr,
		sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	signal(SIGCHLD, fireman);
	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr*) & cli_addr, (socklen_t*)& clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		pid = fork();
		if (pid < 0)
			error("ERROR on fork");
		if (pid == 0) {
			close(sockfd);
			createCode(newsockfd);
			exit(0);
		}
		else close(newsockfd);
	} /* end of while */
	return 0; /* we never get here */
}

void createCode(int sock)
{
	int n;
	char buffer[256];
	char c;

	n = read(sock, buffer, 255);
	if (n < 0) error("ERROR reading from socket");

	int len = strlen(buffer) - 1;
	c = buffer[len];

	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == c)
		{
			buffer[i] = '1';
		}
		else
			buffer[i] = '0';
	}

	n = write(sock, buffer, strlen(buffer)-1);
	if (n < 0) error("ERROR writing to socket");
}