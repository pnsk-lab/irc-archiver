/* $Id$ */

#include "ia_bot.h"

#include "ia_util.h"

#include "ircfw.h"

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int ia_sock;
struct sockaddr_in ia_addr;

bool ia_do_log = false;

void ia_log(const char* txt) {
	if(!ia_do_log) return;
	fprintf(stderr, "%s\n", txt);
}

extern char* host;
extern char* realname;
extern char* nickname;
extern char* username;
extern char* password;
extern char* channels[];
extern int port;

void ia_close(int sock) { close(sock); }

const char* ia_null(const char* str) {
	if(str == NULL) return "(null)";
	return str;
}

bool ia_is_number(const char* str) {
	int i;
	for(i = 0; str[i] != 0; i++) {
		if(!('0' <= str[i] && str[i] <= '9')) return false;
	}
	return true;
}

bool loop = true;

void ia_bot_kill(int sig) {
	ia_log("Shutdown");
	ircfw_socket_send_cmd(ia_sock, NULL, "QUIT :Shutdown (Signal)");
	exit(1);
}

void ia_bot_loop(void) {
	if((ia_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		ia_log("Socket creation failure");
		return;
	}

	loop = true;

	int yes = 1;

	if(setsockopt(ia_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes)) < 0) {
		ia_log("setsockopt failure");
		ia_close(ia_sock);
		return;
	}

	bzero((char*)&ia_addr, sizeof(ia_addr));
	ia_addr.sin_family = PF_INET;
	ia_addr.sin_addr.s_addr = inet_addr(host);
	ia_addr.sin_port = htons(port);

	if(connect(ia_sock, (struct sockaddr*)&ia_addr, sizeof(ia_addr)) < 0) {
		ia_log("Connection failure");
		ia_close(ia_sock);
		return;
	}

	signal(SIGINT, ia_bot_kill);
	signal(SIGTERM, ia_bot_kill);

	char* construct = malloc(1025);

	if(password != NULL && strlen(password) > 0) {
		sprintf(construct, "PASS :%s", password);
		ircfw_socket_send_cmd(ia_sock, NULL, construct);
	}

	sprintf(construct, "USER %s %s %s :%s", username, username, username, realname);
	ircfw_socket_send_cmd(ia_sock, NULL, construct);

	sprintf(construct, "NICK :%s", nickname);
	ircfw_socket_send_cmd(ia_sock, NULL, construct);

	bool is_in = false;

	while(1) {
		int st = ircfw_socket_read_cmd(ia_sock);
		if(st != 0) {
			ia_log("Bad response");
			return;
		}
		if(strlen(ircfw_message.command) == 3 && ia_is_number(ircfw_message.command)) {
			int res = atoi(ircfw_message.command);
			if(!is_in && 400 <= res && res <= 599) {
				ia_log("Bad response");
				return;
			} else if(400 <= res && res <= 599) {
				sprintf(construct, "Ignored error: %d", res);
				ia_log(construct);
				continue;
			}
			if(res == 376) {
				is_in = true;
				ia_log("Login successful");
				int i;
				for(i = 0; channels[i] != NULL; i++) {
					sprintf(construct, "JOIN :%s", channels[i]);
					ircfw_socket_send_cmd(ia_sock, NULL, construct);
				}
			}
		} else {
			if(strcasecmp(ircfw_message.command, "PING") == 0) {
				ia_log("Ping request");
				sprintf(construct, "PONG :%s", ia_null(ircfw_message.prefix));
				ircfw_socket_send_cmd(ia_sock, NULL, construct);
			} else if(strcasecmp(ircfw_message.command, "PRIVMSG") == 0) {
				char* prefix = ircfw_message.prefix;
				char** params = ircfw_message.params;
				if(prefix != NULL && params != NULL) {
					char* nick = ia_strdup(prefix);
					int i;
					for(i = 0; nick[i] != 0; i++) {
						if(nick[i] == '!') {
							nick[i] = 0;
							break;
						}
					}

					free(nick);
				}
			}
		}
	}

	ircfw_socket_send_cmd(ia_sock, NULL, "QUIT :Shutdown");

	free(construct);
}
