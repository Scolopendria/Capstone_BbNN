// Server side C/C++ program to demonstrate Socket programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080
#define Game_Ends 0

void translate(char* board, char* boardAsStr);

int main(int argc, char const* argv[])
{
	int server_fd, new_socket;
	ssize_t valread;
	struct sockaddr_in address;
	int opt = 1;
	socklen_t addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (
setsockopt(
server_fd,
SOL_SOCKET,
			SO_REUSEADDR | SO_REUSEPORT,
&opt,
			sizeof(opt)
)
) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (
bind(
server_fd,
(struct sockaddr*)&address,
			sizeof(address)
) < 0
)  {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0)  {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0
) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	valread = read(new_socket, buffer, 1024 - 1); // subtract 1 for the null terminator at the end
	printf("%s\n", buffer); // So buffer holds what we have received.

	// Intended function is that there are two sockets,
	// (which may not be controlled by different agents)
	// that controls the players

	// This only has one socket
	// Should have two sockets and multiple threads to  listen to multiple channels

	if (!strcmp(buffer, "New Game")) {
		char boardAsStr[10], board[10] = {0};
		translate(board, boardAsStr);
		send(new_socket, boardAsStr, strlen(boardAsStr), 0);
		
		while(1) {
			// Game continues
			if ( Game_Ends ) {
				break;
			}
		}
	}

	send(new_socket, hello, strlen(hello), 0);
	printf("Hello message sent\n");

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	close(server_fd);
	return 0;
}

void translate(char* board, char* boardAsStr) {
	
}
