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

#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

// Create the SSL socket and intialize the socket address structure
int OpenListener(int port)
{
	int sd;
	struct sockaddr_in addr;
	sd = socket(PF_INET, SOCK_STREAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) {
		perror("can't bind port");
		abort();
	}
	if ( listen(sd, 10) != 0 ) {
		perror("Can't configure listening port");
		abort();
	}
	return sd;
}

SSL_CTX* InitServerCTX(void)
{
	OpenSSL_add_all_algorithms();  /* load & register all cryptos. */
	SSL_load_error_strings();      /* load all error messages */
	const SSL_METHOD *method
		= TLS_server_method(); /* create new server-method */
	SSL_CTX *ctx
		= SSL_CTX_new(method); /* create new context from method */
	if (!ctx) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
	/* set the local certificate from CertFile */
	if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 ) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 ) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	/* verify private key */
	if ( !SSL_CTX_check_private_key(ctx) ) {
		fprintf(stderr, "Private key does not match the public certificate\n");
		abort();
	}
}
void ShowCerts(const SSL* ssl)
{
	/* Get certificates (if available) */
	X509 *cert = SSL_get_peer_certificate(ssl);

	if (cert) {
		printf("Server certificates:\n");
		char *line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Subject: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	} else
		printf("No certificates.\n");
}
void Servlet(SSL* ssl) /* Serve the connection -- threadable */
{
	char buf[1024] = {0};

	const char* ServerResponse=
		"<body>\n"
		"<name>aticleworld.com</name>\n"
		"<year>1.5</year>"
		"<blogType>Embedded and c/c++</blogType>\n"
		"<author>amlendra</author>\n"
		"</body>";

	const char *cpValidMessage =
		"<Body>\n"
		"<UserName>jimmy</UserName>\n"
		"<Password>1234</Password>\n"
		"</Body>";
	if ( SSL_accept(ssl) == -1 )     /* do SSL-protocol accept */
		ERR_print_errors_fp(stderr);
	else {
		ShowCerts(ssl);          /* get any certificates */
		int bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
		buf[bytes] = '\0';
		printf("Client msg: --\n %s\n--\n", buf);

		if (bytes > 0) {
			if (strcmp(cpValidMessage, buf) == 0)
				SSL_write(ssl, ServerResponse, strlen(ServerResponse)); /* send reply */
			else
				SSL_write(ssl, "Invalid Message", strlen("Invalid Message")); /* send reply */
		} else
			ERR_print_errors_fp(stderr);
	}

	int sd = SSL_get_fd(ssl); /* get socket connection */
	SSL_free(ssl);            /* release SSL state */
	close(sd);                /* close connection */
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage: %s <portnum>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int portnum = atoi(argv[1]);

	if (portnum < 1024 && getuid() != 0) {
		printf("Port %d needs root/sudo user!!", portnum);
		exit(EXIT_FAILURE);
	}

	// Initialize the SSL library
	SSL_library_init();
	SSL_CTX *ctx = InitServerCTX();     /* initialize SSL */

	LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
	int server = OpenListener(portnum); /* create server socket */

	while (1) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);

		int client =             /* accept connection as usual */
			accept(server, (struct sockaddr*)&addr, &len);
		printf("Connection: %s:%d\n",
		       inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		SSL *ssl = SSL_new(ctx); /* get new SSL state with context */
		SSL_set_fd(ssl, client); /* set connection socket to SSL state */
		Servlet(ssl);            /* service connection */
	}

	close(server);          /* close server socket */
	SSL_CTX_free(ctx);      /* release context */
	return 0;
}
