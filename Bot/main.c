/* $Id$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "ia_util.h"
#include "ia_bot.h"

extern bool ia_do_log;

char* host = NULL;
int port = 0;
char* nickname = NULL;
char* username = NULL;
char* realname = NULL;
char* password = NULL;
char* admin = NULL;
char* channels[128];
int chanincr;

int main(int argc, char** argv) {
	const char* fn = "archiver.ini";
	int i;
	bool daemon = true;
	for(i = 1; i < argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-D") == 0) {
				ia_do_log = true;
				daemon = false;
			} else {
				fprintf(stderr, "Unknown option: %s\n", argv[i]);
				return 1;
			}
		} else {
			fn = argv[i];
		}
	}

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

	int incr = 0;

	channels[0] = NULL;

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
						} else if(strcmp(key, "nickname") == 0) {
							if(nickname != NULL) free(nickname);
							nickname = ia_strdup(value);
						} else if(strcmp(key, "password") == 0) {
							if(password != NULL) free(password);
							password = ia_strdup(value);
						} else if(strcmp(key, "admin") == 0) {
							if(admin != NULL) free(admin);
							admin = ia_strdup(value);
						} else if(strcmp(key, "realname") == 0) {
							if(realname != NULL) free(realname);
							realname = ia_strdup(value);
						} else if(strcmp(key, "channel") == 0) {
							channels[chanincr++] = ia_strdup(value);
							channels[chanincr] = NULL;
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
	if(nickname == NULL) {
		fprintf(stderr, "Specify nickname\n");
		st = 1;
	}
	if(password == NULL) {
		fprintf(stderr, "Specify password\n");
		st = 1;
	}
	if(admin == NULL) {
		fprintf(stderr, "Specify admin\n");
		st = 1;
	}
	if(realname == NULL) {
		fprintf(stderr, "Specify realname\n");
		st = 1;
	}
	if(st == 1) return st;

	printf("Bot spawning a daemon\n");
	pid_t pid = 0;
	if(daemon) {
		pid = fork();
	}
	if(pid == 0) {
		ia_bot_loop();
		_exit(0);
	} else {
		printf("Spawned daemon, I am exiting\n");
	}

	if(host != NULL) free(host);
	if(realname != NULL) free(realname);
	if(username != NULL) free(username);
	if(nickname != NULL) free(nickname);
	if(password != NULL) free(password);
	if(admin != NULL) free(admin);
	for(i = 0; channels[i] != NULL; i++) {
		free(channels[i]);
	}
}
