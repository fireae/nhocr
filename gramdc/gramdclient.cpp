/*
 * gramdclient.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: asanoki
 */

#include "gramdclient.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <unistd.h>

#define GRAMD_CLIENT_BUFFER_SIZE 65536
#define TCP_NODELAY 1

static const char *gramd_error_message = NULL;
static int gramd_error_id = 0;
static int gramd_debug_mode = 0;

void gramdDebugMode(int mode) {
	gramd_debug_mode = mode;
}

void gramdInfo(const char *message) {
	if (gramd_debug_mode > 1) {
		perror(message);
	}
}

void gramdError(int id, const char *message) {
	if (gramd_debug_mode > 0) {
		perror(message);
	}
	gramd_error_message = message;
	gramd_error_id = id;
}

gramd_connection *gramdOpen(char *hostname, unsigned short port) {
	gramdInfo("Creating socket...");

	int handle = socket(AF_INET, SOCK_STREAM, 0);
	if (handle < 0)
		return NULL;

	int flag = 1;
	setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));

	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));

	server.sin_family = AF_INET;
	struct hostent *hp;

	gramdInfo("Resolving hostname...");
	if ((hp = gethostbyname(hostname)) == NULL) {
		gramdError(10, "Unable to create socket.");
		close(handle);
		return NULL;
	}

	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(port);

	gramdInfo("Connecting...");
	if (connect(handle, (struct sockaddr *) &server, sizeof(server)) < 0) {
		gramdError(11, "Unable to connect to the service.");
		close(handle);
		return NULL;
	}

	gramdInfo("Connected.");
	gramd_connection *connection = new gramd_connection();
	connection->socket = handle;
	connection->result_buffer = (char *) malloc(GRAMD_CLIENT_BUFFER_SIZE);
	connection->result_conversion_buffer = (char *) malloc(
			GRAMD_CLIENT_BUFFER_SIZE);
	connection->result_buffer[0] = 0;
	connection->result_buffer_it = connection->result_buffer;

	return connection;
}

gramd_connection *gramdOpenWithFile(char *port_filename, char *hostname) {
	gramdInfo("Opening with file...");
	FILE *file = fopen(port_filename, "r");
	if (!file) {
		gramdError(15, "Unable to open the file with a port information.");
		return NULL;
	}
	char buffer[32];
	fgets(buffer, 32, file);
	unsigned short port = atoi(buffer);
	fclose(file);
	return gramdOpen(hostname, port);
}

int gramdAddSection(gramd_connection *connection) {
	gramdInfo("Adding section.");
	char buffer[16] = "\n";
	return send(connection->socket, buffer, strlen(buffer), 0);
}

int gramdAddCandidate(gramd_connection *connection, char *label,
		double probability) {
	gramdInfo("Adding candidate.");
	char buffer[64];
	snprintf(buffer, 64, "%s\t%f\n", label, probability);
	return send(connection->socket, buffer, strlen(buffer), 0);
}

int gramdAddWCandidate(gramd_connection *connection, wchar_t label,
		double probability) {
	gramdInfo("Adding candidate using wchar_t.");
	char buffer[64];
	mbstowcs(&label, buffer, 1);
	return gramdAddCandidate(connection, buffer, probability);
}

int gramdQuery(gramd_connection *connection, char *output, size_t size,
		int timeout) {
	gramdInfo("Performing query...");
	gramdAddSection(connection);
	gramdAddSection(connection);
	fd_set set;
	struct timeval to;

	output[0] = 0;

	gramdInfo("Initializing the file descriptor set...");
	/* Initialize the file descriptor set. */
	FD_ZERO(&set);
	FD_SET((unsigned int)connection->socket, &set);
	to.tv_sec = timeout / 1000;
	to.tv_usec = 1000 * (timeout % 1000);

	gramdInfo("Starting while loop...");
	while (true) {
		int select_result;
		if ((select_result = select(connection->socket + 1, &set, NULL, NULL,
				&to)) <= 0) {
			if (select_result < 0)
				gramdError(20, "Connection broken.");
			else
				gramdError(21, "Connection timeouted.");
			return 0;
		}
		gramdInfo("Got a bunch of data...");
		char *previous_it = connection->result_buffer_it;
		if (GRAMD_CLIENT_BUFFER_SIZE
				- (connection->result_buffer_it - connection->result_buffer_it)
				- 1 <= 0) {
			gramdError(22, "Internal buffer too small.");
			return 0;
		}
		int result = recv(
				connection->socket,
				(char *) connection->result_buffer_it,
				GRAMD_CLIENT_BUFFER_SIZE
						- (connection->result_buffer_it
								- connection->result_buffer_it) - 1, 0);
		if (result < 0) {
			gramdError(23, "Connection broken.");
			return 0;
		}
		connection->result_buffer_it += result;
		*connection->result_buffer_it = 0;

		char *line_pos = strstr(previous_it, "\n");
		if (line_pos != NULL) {
			/// line_pos indicates \n
			// Found a new-line character.
			gramdInfo("Found a line, parsing...");
			*line_pos = 0;
			size_t line_length = line_pos - connection->result_buffer;
			if (line_length + 1 > size) {
				gramdError(
						24,
						"Provided buffer for is too small to handle the response.");
				return 0;
			}
			strncat(output, connection->result_buffer, line_length);
//			gramdInfo("Result is:");
//			printf("%d: %s\n", line_length, output);
			size_t to_move = (connection->result_buffer_it
					- connection->result_buffer /* size with \n */)
					- line_length - 1 /* \n */+ 1 /* \0 */;
			memmove(connection->result_buffer, line_pos + 1, to_move);
			connection->result_buffer_it = connection->result_buffer + to_move
					- 1;
			return line_length;
		}

	}
	return 0;
}

int gramdWQuery(gramd_connection *connection, wchar_t *output, size_t size,
		int timeout) {
	connection->result_conversion_buffer[0] = 0;
	output[0] = 0;
	int result = gramdQuery(connection, connection->result_conversion_buffer,
			size, timeout);
	if (result + 1 > (int) size) {
		gramdError(30,
				"Provided buffer may be too too small to handle the response.");
		return 0;
	}
	return mbstowcs(output, connection->result_conversion_buffer, result);
}

int gramdClose(gramd_connection *connection) {
	delete[] connection->result_buffer;
	delete[] connection->result_conversion_buffer;
	close(connection->socket);
	delete connection;
	return 1;
}
