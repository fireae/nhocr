/*
 * gramdclient.h
 *
 *  Created on: Mar 19, 2012
 *      Author: asanoki
 */

#ifndef GRAMDCLIENT_H_
#define GRAMDCLIENT_H_

#include <stdlib.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef struct gramd_connection {
	char *result_conversion_buffer;
	char *result_buffer;
	char *result_buffer_it;
	int socket;
} gramd_connection;

void gramdDebugMode(int mode);

gramd_connection *gramdOpen(char *hostname, unsigned short port);
gramd_connection *gramdOpenWithFile(char *port_filename, char *hostname);

int gramdAddSection(gramd_connection *connection);
int gramdAddCandidate(gramd_connection *connection, char *label,
		double probability);
int gramdAddWCandidate(gramd_connection *connection, wchar_t *label,
		double probability);
int gramdQuery(gramd_connection *connection, char *output, size_t size,
		int timeout);
int gramdWQuery(gramd_connection *connection, wchar_t *output, size_t size,
		int timeout);

int gramdClose(gramd_connection *connection);

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif

#endif /* GRAMDCLIENT_H_ */
