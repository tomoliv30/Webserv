#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

namespace ft{

std::string	strip(std::string s, char const c) {
	std::size_t pos;

	if ((pos = s.find_first_not_of(c)) != std::string::npos) s.erase(0, pos);
	if ((pos = s.find_last_not_of(c)) != std::string::npos) s.erase(pos + 1);

	return s;
}

std::vector<std::string>	split(std::string s, char c) {
	std::vector<std::string>	v;
	std::size_t	pos = 0;
	std::string	token;

	s = ft::strip(s, c);

	while ((pos = s.find_first_of(c)) != std::string::npos) {
		token = s.substr(0, pos);
		v.push_back(token);
		s.erase(0, pos + 1);
	}
	v.push_back(s);
	return v;
}

std::string itoa(int n) {
	std::stringstream out;
	out << n;
	return out.str();
}

bool file_exists(std::string const & filename) {
	struct stat	buffer;
	return stat(filename.c_str(), &buffer) == 0;
}

bool is_big_endian() {
	union {
		uint32_t i;
		uint8_t	 c[4];
	} u = {0x01020304};

	return u.c[0] == 1;
}

uint32_t hostToNetworkLong(uint32_t hostlong) {
	if (ft::is_big_endian())
		return hostlong;
	return (hostlong & 0x000000FF) << 24 |
		   (hostlong & 0x0000FF00) << 8 |
		   (hostlong & 0x00FF0000) >> 8 |
		   (hostlong & 0xFF000000) >> 24;
}

uint16_t hostToNetworkShort(uint16_t hostshort) {
	if (ft::is_big_endian())
		return hostshort;
	return (hostshort & 0x00FF) << 8 | (hostshort & 0xFF00) >> 8;
}

int sendall(int socket, const char *buffer, size_t & length) {
	size_t total = 0;
	size_t bytesleft = length;
	size_t n;

	while (total < length) {
		n = send(socket, buffer + total, bytesleft, 0);
		if (n == -1)
			break ;
		total += n;
		bytesleft -= n;
	}

	length = total;

	return n == -1 ? -1 : 0;
}

}

std::map<int, std::string>	status_codes;
std::map<std::string, std::string>	mime_types;

std::string responseLine(int status_code) {
	std::string response_line = "HTTP/1.1 ";

	response_line.append(ft::itoa(status_code));
	response_line.append(" ");
	response_line.append(status_codes[status_code]);
	response_line.append("\r\n");

	return response_line;
}

std::string mimeTypes(std::string const & filename) {
	std::string mime_type = "Content-Type: ";
	std::size_t pos;

	pos = filename.find_last_of('.');

	if (pos == std::string::npos) {
		mime_type += "text/plain";
	} else {
		mime_type += mime_types[filename.substr(pos + 1)];
	}

	mime_type.append("\r\n");

	return mime_type;
}

std::string responseHeaders(std::string const & filename, std::size_t length) {
	std::string response_headers = "Server: webserv\r\n";

	response_headers += mimeTypes(filename);

	response_headers += "Content-Length: ";
	response_headers += ft::itoa(length);
	response_headers.append("\r\n");

	return response_headers;
}

std::string responseBody(std::string const & filename) {
	std::ifstream		in(filename);
	std::stringstream	out;
	out << in.rdbuf();

	return out.str();
}

std::string handle_GET(std::vector<std::string> const & request) {
	std::string filename = ft::strip(request[1], '/');
	if (filename == "/") filename = "index.html";

	std::string	response_line, response_headers, response_body;

	if (ft::file_exists(filename)) {
		response_line = responseLine(200);
		response_body = responseBody(filename);
		response_headers = responseHeaders(filename, response_body.length());
	} else {
		response_line = responseLine(404);
		response_body = responseBody("error.html");
		response_headers = responseHeaders("error.html", response_body.length());
	}

	return response_line + response_headers + "\r\n" + response_body;
}

std::string handle_HEAD(std::vector<std::string> const & request) {
	std::string filename = ft::strip(request[1], '/');
	if (filename == "/") filename = "index.html";

	std::string	response_line, response_headers;

	if (ft::file_exists(filename)) {
		response_line = responseLine(200);
		response_headers = responseHeaders(filename, 0);
	} else {
		response_line = responseLine(404);
		response_headers = responseHeaders("404.html", 0);
	}

	return response_line + response_headers + "\r\n";
}

std::string handle_DELETE(std::vector<std::string> const & request) {
	std::string filename = ft::strip(request[1], '/');

	std::string response_line, response_headers;

	if (ft::file_exists(filename)) {
		response_line = responseLine(200);
		response_headers = "Server: webserv\r\n";
		std::cout << "DELETING " << filename << std::endl;
//		unlink(filename.c_str());
	} else {
		response_line = responseLine(404);
		response_headers = "Server: webserv\r\n";
	}
	return response_line + response_headers + "\r\n";
}

std::string HTTP_501_handler(std::vector<std::string> const & request) {
	std::string response_line = responseLine(501);
	std::string response_headers = "Server: webserv\r\nContent-Type: text/html\r\nContent-Length: 28\r\n";
	std::string response_body = "<h1>501 Not Implemented</h1>";

	return response_line + response_headers + "\r\n" + response_body;
}

int	main(int argc, char **argv) {
	struct	sockaddr_in	address;

	int			server_fd;
	int			new_socket;
	int			addrlen = sizeof(address);
	long		valread;
	char		buffer[30000] = {0};

	fd_set	read_fds, read_master, write_fds, write_master;
	int	fd_max;

	FD_ZERO(&read_fds);
	FD_ZERO(&read_master);
	FD_ZERO(&write_fds);
	FD_ZERO(&write_master);

	status_codes[100] = "Continue";
	status_codes[101] = "Switching Protocols";

	status_codes[200] = "OK";
	status_codes[201] = "Created";
	status_codes[202] = "Accepted";
	status_codes[203] = "Non-Authorative Information";
	status_codes[204] = "No Content";
	status_codes[205] = "Reset Content";
	status_codes[206] = "Partial Content";

	status_codes[300] = "Multiple Choices";
	status_codes[301] = "Moved Permanently";
	status_codes[302] = "Found";
	status_codes[303] = "See Other";
	status_codes[304] = "Not Modified";
	status_codes[305] = "Use Proxy";
	status_codes[307] = "Temporary Redirect";

	status_codes[400] = "Bad Request";
	status_codes[401] = "Unauthorized";
	status_codes[402] = "Payment Required";
	status_codes[403] = "Forbidden";
	status_codes[404] = "Not Found";
	status_codes[405] = "Method Not Allowed";
	status_codes[406] = "Not Acceptable";
	status_codes[407] = "Proxy Authentication Required";
	status_codes[408] = "Request Timeout";
	status_codes[409] = "Conflict";
	status_codes[410] = "Gone";
	status_codes[411] = "Length Required";
	status_codes[412] = "Precondition Failed";
	status_codes[413] = "Payload Too Large";
	status_codes[414] = "URI Too Long";
	status_codes[415] = "Unsupported Media Type";
	status_codes[416] = "Range Not Satisfiable";
	status_codes[417] = "Expectation Failed";
	status_codes[426] = "Upgrade Required";

	status_codes[500] = "Internal Server Error";
	status_codes[501] = "Not Implemented";
	status_codes[502] = "Bad Gateway";
	status_codes[503] = "Service Unavailable";
	status_codes[504] = "Gateway Timeout";
	status_codes[505] = "HTTP Version Not Supported";

	mime_types["html"] = "text/html";
	mime_types["css"] = "text/css";
	mime_types["mp4"] = "video/mp4";
	mime_types["png"] = "image/png";

	std::map<std::string, std::string (*)(std::vector<std::string> const &)>	methods;

	methods["GET"] = handle_GET;
	methods["HEAD"] = handle_HEAD;
//	methods["POST"];
//	methods["PUT"];
	methods["DELETE"] = handle_DELETE;
//	methods["CONNECT"];
//	methods["OPTIONS"];
//	methods["TRACE"];

//	HTTPServer	s;

	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int enable = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
//	address.sin_addr.s_addr = inet_addr("192.168.192.23");
//	address.sin_addr.s_addr = inet_addr("0.0.0.0");
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = ft::hostToNetworkShort(8080);
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));

	if (bind(server_fd,  reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 10) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	FD_SET(server_fd, &read_master);
	fd_max = server_fd;

	while (true) {
		read_fds = read_master;
		write_fds = write_master;
		if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i <= fd_max; ++i) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == server_fd) {
					new_socket = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address),
										reinterpret_cast<socklen_t *>(&addrlen));
					if (new_socket == -1) {
						perror("accept");
					} else {
						FD_SET(new_socket, &read_master);
//						fcntl(new_socket, F_SETFL, O_NONBLOCK);
						if (new_socket > fd_max)
							fd_max = new_socket;
					}
				} else {
					memset(buffer, 0, sizeof(buffer));
					valread = recv(i, buffer, 30000, 0);

					if (valread <= 0) {
						close(i);
						FD_CLR(i, &read_master);
					} else {
						std::string	request_line(buffer);

						request_line = request_line.substr(0, request_line.find_first_of("\r\n"));

						std::vector<std::string>	request = ft::split(request_line, ' ');

						std::string response;
						if (methods.find(request[0]) != methods.end()) {
							response = methods[request[0]](request);
						} else {
							response = HTTP_501_handler(request);
						}

						size_t length = response.length();
						ft::sendall(i, response.c_str(), length);
					}
				}
			}
		}
	}
	return 0;
}
