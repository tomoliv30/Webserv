#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

int main() {
	struct sockaddr_in	address;

	int sock_fd;
	char buffer[30000] = {0};

	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(8080);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	if (connect(sock_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	std::string	s = "DELETE /delete_test HTTP/1.1\r\nHost: localhost\r\n\r\n";

	send(sock_fd, s.c_str(), s.length(), 0);

	recv(sock_fd, buffer, 30000, 0);

	std::cout << buffer;

	return 0;
}
