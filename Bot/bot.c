/* $Id$ */

#include "ia_bot.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

int ia_sock;
struct sockaddr_in is_addr;

bool ia_do_log = false;

void ia_log(const char* txt) {
	if(!ia_do_log) return;
	fprintf(stderr, "%s\n", txt);
}

extern char* host;
extern int port;

void ia_bot_loop(void) {
	if((ia_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		ia_log("Socket creation failure");
		return;
	}

	int yes = 1;

	if(setsockopt(ia_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&yes, sizeof(yes)) < 0) {
		ia_log("setsockopt failure");
		return;
	}

	bzero((char*)&is_addr, sizeof(is_addr));
	is_addr.sin_family = PF_INET;
	is_addr.sin_addr.s_addr = inet_addr(host);
	is_addr.sin_port = htons(port);

	if(connect(ia_sock, (struct sockaddr*)&ia_addr, sizeof(ia_addr)) < 0) {
		ia_log("Connection failure");
		close(ia_sock);
		return 1;
	}

	while(1) {
	}
}
