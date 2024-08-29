/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ia_util.h"
#include "ia_util.h"

int main(int argc, char** argv) {
	const char* fn = argv[1] == NULL ? "archiver.ini" : argv[1];
	FILE* f = fopen(fn, "r");
	if(f == NULL) {
		fprintf(stderr, "Could not open the config: %s\n", fn);
		return 1;
	}

	struct stat s;
	stat(fn, &s);

	char* buf = malloc(s.st_size + 1);
	fread(buf, s.st_size, 1, f);
	buf[s.st_size] = 0;

	int i;
	int incr = 0;

	char* host = NULL;
	int port = 0;
	char* username = NULL;
	char* password = NULL;

	for(i = 0;; i++) {
		if(buf[i] == 0 || buf[i] == '\n') {
			char oldc = buf[i];
			buf[i] = 0;
			char* line = buf + incr;
			if(strlen(line) > 0 && line[0] != '#') {
				int j;
				for(j = 0; line[j] != 0; j++) {
					if(line[j] == '=') {
						line[j] = 0;
						char* key = line;
						char* value = line + j + 1;

						if(strcmp(key, "host") == 0) {
							if(host != NULL) free(host);
							host = ia_strdup(value);
						} else if(strcmp(key, "port") == 0) {
							port = atoi(value);
						} else if(strcmp(key, "username") == 0) {
							if(username != NULL) free(username);
							username = ia_strdup(value);
						} else if(strcmp(key, "password") == 0) {
							if(password != NULL) free(password);
							password = ia_strdup(value);
						}

						break;
					}
				}
			}
			incr = i + 1;
			if(oldc == 0) break;
		}
	}

	free(buf);
	fclose(f);

	int st = 0;
	if(host == NULL) {
		fprintf(stderr, "Specify host\n");
		st = 1;
	}
	if(username == NULL) {
		fprintf(stderr, "Specify username\n");
		st = 1;
	}
	if(password == NULL) {
		fprintf(stderr, "Specify password\n");
		st = 1;
	}
	if(st == 1) return st;

	if(host != NULL) free(host);
	if(username != NULL) free(username);
	if(password != NULL) free(password);
}
