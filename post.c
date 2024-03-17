/*
 * Copyright (C) 2019  Jimmy Aguilar Mena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit, atoi, malloc, free */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <arpa/inet.h>
#include <netdb.h> /* struct hostent, gethostbyname */
#include <errno.h>

#define BUFFSIZE 40960

void error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc,char *argv[])
{
	char msgsend[] =
		"POST /test.php HTTP/1.1\r\n"
		"Host: 192.167.1.40:8080\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: 25\r\n"
		"\r\n"
		"uname=jimmy&password=1234\r\n"
		;

	char msgrecv[BUFFSIZE] = {0};

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Socket creation error\n");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "192.168.1.40", &serv_addr.sin_addr) <= 0) {
		perror("Invalid address Address not supported\n");
		exit(EXIT_FAILURE);
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))
	    < 0) {
		perror("Connection Failed\n");
		exit(EXIT_FAILURE);
	}

	int nsend = send(sock , msgsend , strlen(msgsend) , 0);
	if (nsend < 0) {
		const int errnum = errno;
		fprintf(stderr, "Send error: %d\n", errnum);
		fprintf(stderr, "Error message sending: %s\n",
		        strerror(errnum));
	}

	int nrecv = read(sock , msgrecv, BUFFSIZE);
	printf("%s\n", msgrecv);

	return 0;
}
