/* $Id$ */

#include "ia_bot.h"

#include "ia_util.h"
#include "ia_db.h"

#include "web_html.h"

#include "ircfw.h"

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define IRCARC_VERSION "1.01"

const char* ircarc_version = IRCARC_VERSION;

int ia_sock;
struct sockaddr_in ia_addr;

bool ia_do_log = false;

void ia_log(const char* txt) {
	if(!ia_do_log) return;
	fprintf(stderr, "%s\n", txt);
}

extern char* host;
extern char* admin;
extern char* realname;
extern char* nickname;
extern char* username;
extern char* password;
extern char* nickserv;
extern char* webroot;
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

	while(loop) {
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
				sprintf(construct, "NOTICE %s :IRC-Archiver %s is ready to accept requests", admin, IRCARC_VERSION);
				ircfw_socket_send_cmd(ia_sock, NULL, construct);
				if(nickserv != NULL) {
					sprintf(construct, "PRIVMSG NickServ :%s", nickserv);
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
				int i;
				int len = 0;
				if(params != NULL) {
					for(i = 0; params[i] != NULL; i++)
						;
					len = i;
				}
				if(prefix != NULL && len == 2) {
					char* sentin = params[0];
					char* msg = params[1];
					char* nick = ia_strdup(prefix);
					for(i = 0; nick[i] != 0; i++) {
						if(nick[i] == '!') {
							nick[i] = 0;
							break;
						}
					}

					if(msg[0] == 1 && msg[strlen(msg) - 1] == 1) {
						/* CTCP */
						if(strcasecmp(msg, "\x01VERSION\x01") == 0) {
							sprintf(construct, "NOTICE %s :\x01VERSION IRC-Archiver %s / IRC Frameworks %s: http://nishi.boats/ircarc\x01", nick, IRCARC_VERSION, IRCFW_VERSION);
							ircfw_socket_send_cmd(ia_sock, NULL, construct);
						}
					} else if(sentin[0] == '#') {
						/* This was sent in channel ; log it */
						ia_db_put(nick, sentin, msg);
					} else {
						/* Command, I guess */
						int i;

						char* name = NULL;
						char* chn = NULL;
						char* frm = NULL;
						char* to = NULL;

						char* prm = ia_strdup(msg);
						int incr = 0;
						int t = 0;
						for(i = 0;; i++) {
							if(prm[i] == ' ' || prm[i] == 0) {
								char oldc = prm[i];
								prm[i] = 0;

								if(strcasecmp(prm, "help") == 0) {
									sprintf(construct, "PRIVMSG %s :HELP   Show this help", nick);
									ircfw_socket_send_cmd(ia_sock, NULL, construct);
									sprintf(construct, "PRIVMSG %s :SHUTDOWN   Shutdown the bot", nick);
									ircfw_socket_send_cmd(ia_sock, NULL, construct);
									sprintf(construct, "PRIVMSG %s :ARCHIVE [name] [channel] <yyyy-mm-dd-hh:mm:ss> <yyyy-mm-dd-hh:mm:ss>   Archive the log", nick);
									ircfw_socket_send_cmd(ia_sock, NULL, construct);
								} else if(strcasecmp(prm, "archive") == 0) {
									if(strcmp(admin, nick) == 0) {
										if(t <= 4 && t != 0) {
											char* p = prm + incr;
											if(t == 1) {
												name = p;
											} else if(t == 2) {
												chn = p;
											} else if(t == 3) {
												frm = p;
											} else if(t == 4) {
												to = p;
											}
										}
										if(oldc == 0) {
											if(chn == NULL) {
												sprintf(construct, "PRIVMSG %s :Insufficient arguments", nick);
												ircfw_socket_send_cmd(ia_sock, NULL, construct);
											} else if(strcmp(name, "index") == 0) {
												sprintf(construct, "PRIVMSG %s :You cannot use that name", nick);
												ircfw_socket_send_cmd(ia_sock, NULL, construct);
											} else {
												web_range_t range;
												struct tm from_tm;
												memset(&from_tm, 0, sizeof(from_tm));
												struct tm to_tm;
												memset(&to_tm, 0, sizeof(to_tm));
												time_t tfrom = 0;
												time_t tto = 0;
												if(frm != NULL) {
													if(strptime(frm, "%Y/%m/%d-%H:%M:%S", &from_tm) == NULL) {
														sprintf(construct, "PRIVMSG %s :Date parsing failure", nick);
														ircfw_socket_send_cmd(ia_sock, NULL, construct);
														break;
													}
													tfrom = mktime(&from_tm);
												}
												if(to != NULL) {
													if(strptime(to, "%Y/%m/%d-%H:%M:%S", &to_tm) == NULL) {
														sprintf(construct, "PRIVMSG %s :Date parsing failure", nick);
														ircfw_socket_send_cmd(ia_sock, NULL, construct);
														break;
													}
													tto = mktime(&to_tm);
												}

												char* hpath = ia_strcat4(webroot, "/", name, ".html");
												struct stat s;
												if(stat(hpath, &s) == 0) {
													sprintf(construct, "PRIVMSG %s :Archive which has the same name already exists", nick);
													ircfw_socket_send_cmd(ia_sock, NULL, construct);
													break;
												}

												int j;
												bool found = false;
												for(j = 0; channels[j] != NULL; j++) {
													if(strcmp(channels[j], chn) == 0) {
														found = true;
														break;
													}
												}
												if(!found) {
													sprintf(construct, "PRIVMSG %s :I do not know the channel", nick);
													ircfw_socket_send_cmd(ia_sock, NULL, construct);
													break;
												}
												range.from = tfrom;
												range.to = tto;
												range.channel = chn;
												web_html_generate(name, range);
											}
										}
									} else {
										sprintf(construct, "PRIVMSG %s :You must be an admin to use this", nick);
										ircfw_socket_send_cmd(ia_sock, NULL, construct);
										break;
									}
								} else if(strcasecmp(prm, "shutdown") == 0) {
									if(strcmp(admin, nick) == 0) {
										loop = false;
									} else {
										sprintf(construct, "PRIVMSG %s :You must be an admin to use this", nick);
										ircfw_socket_send_cmd(ia_sock, NULL, construct);
									}
									break;
								} else {
									sprintf(construct, "PRIVMSG %s :Unknown command `%s'. See HELP.", nick, prm);
									ircfw_socket_send_cmd(ia_sock, NULL, construct);
									break;
								}

								incr = i + 1;
								t++;
								if(oldc == 0) break;
							}
						}
						free(prm);
					}

					free(nick);
				}
			}
		}
	}

	ircfw_socket_send_cmd(ia_sock, NULL, "QUIT :Shutdown");

	free(construct);
}
