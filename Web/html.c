/* $Id$ */

#include "web_html.h"

#include "web_db.h"

#include "ia_util.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern char* webroot;
extern const char* ircarc_version;

char* web_html_escape(const char* html) {
	char* str = malloc(strlen(html) * 5 + 1);
	int i;
	int incr = 0;
	for(i = 0; html[i] != 0; i++) {
		if(html[i] == '&') {
			str[incr++] = '&';
			str[incr++] = 'a';
			str[incr++] = 'm';
			str[incr++] = 'p';
			str[incr++] = ';';
		} else if(html[i] == '<') {
			str[incr++] = '&';
			str[incr++] = 'l';
			str[incr++] = 't';
			str[incr++] = ';';
		} else if(html[i] == '>') {
			str[incr++] = '&';
			str[incr++] = 'g';
			str[incr++] = 't';
			str[incr++] = ';';
		} else {
			str[incr++] = html[i];
			;
		}
	}
	str[incr] = 0;
	return str;
}

#define TAG(tagname, el, chr) \
	bool attr = true; \
	if(bufincr > 0) { \
		int k; \
		for(k = bufincr - 1; k >= 0; k--) { \
			if(buffer[k] == chr) { \
				attr = false; \
				int l; \
				buffer[k] = 0; \
				for(l = k; l < bufincr; l++) { \
					buffer[l] = buffer[l + 1]; \
				} \
				break; \
			} \
		} \
		if(!attr) bufincr--; \
	} \
	if(attr) { \
		buffer[bufincr++] = chr; \
		char* tmp = fmtmsg; \
		fmtmsg = ia_strcat4(tmp, "<" tagname, el, ">"); \
		free(tmp); \
	} else { \
		char* tmp = fmtmsg; \
		fmtmsg = ia_strcat(tmp, "</" tagname ">"); \
		free(tmp); \
	}

int web_html_generate(const char* name, web_range_t range) {
	time_t t = time(NULL);
	char* path = ia_strcat4(webroot, "/", name, ".html");
	FILE* f = fopen(path, "w");
	if(f != NULL) {
		entry_t** e = web_db_query(range);

		char date[512];
		struct tm* tm = gmtime(&t);
		strftime(date, sizeof(date), "%Y/%m/%d %H:%M:%S UTC", tm);
		char* htmlesc = web_html_escape(name);
		char* title = ia_strcat("Archive: ", htmlesc);
		free(htmlesc);
		fprintf(f, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
		fprintf(f, "<html>\n");
		fprintf(f, "	<head>\n");
		fprintf(f, "		<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\n");
		fprintf(f, "		<title>%s</title>\n", title);
		fprintf(f, "		<style type=\"text/css\">\n");
		fprintf(f, "		</style>\n");
		fprintf(f, "	</head>\n");
		fprintf(f, "	<body style=\"padding: 15px; margin: 0 auto; width: 900px;\">\n");
		fprintf(f, "		<div style=\"padding: 1px 0; margin: 0; text-align: center; background-color: #8080ff;\">\n");
		fprintf(f, "			<h1>%s</h1>\n", title);
		fprintf(f, "		</div>\n");
		fprintf(f, "		Archived at %s.<br>\n", date);
		htmlesc = web_html_escape(range.channel);
		fprintf(f, "		Channel: <code>%s</code><br>\n", htmlesc);
		free(htmlesc);
		fprintf(f, "		<hr>\n");
		fprintf(f, "		<a href=\"./\">Go back to index</a>\n");
		fprintf(f, "		<hr>\n");
		fprintf(f, "<div>\n");
		fprintf(f, "		<pre style=\"margin: 0; border: solid 1px black; padding: 10px; overflow: scroll; width: 678px; min-height: 900px; background-color: #d0d0ff; float: left;\"><code>");
		int i;
		int bgcolor = 0xd0d0ff;
		int fgcolor = 0x000000;
		for(i = 0; e[i] != NULL; i++) {
			time_t t = e[i]->time;
			struct tm* tm = gmtime(&t);
			char* escusr = web_html_escape(e[i]->username);
			char* escmsg = web_html_escape(e[i]->message);
			char date[512];
			strftime(date, 512, "%Y/%m/%d %H:%M:%S UTC", tm);
			char cbuf[2];
			cbuf[1] = 0;
			char* fmtmsg = ia_strdup("");
			int j;
			char buffer[512];
			memset(buffer, 0, 512);
			int bufincr = 0;
			for(j = 0; escmsg[j] != 0; j++) {
				if(escmsg[j] == 2) {
					TAG("b", "", 'B');
				} else if(escmsg[j] == 0x1d) {
					TAG("i", "", 'I');
				} else if(escmsg[j] == 0x1f) {
					TAG("u", "", 'U');
				} else if(escmsg[j] == 0x1e) {
					TAG("s", "", 'S');
				} else if(escmsg[j] == 0x16) {
					char fgt[32];
					char bgt[32];
					int _c = fgcolor;
					fgcolor = bgcolor;
					bgcolor = _c;
					sprintf(fgt, "#%06X", fgcolor);
					sprintf(bgt, "#%06X", bgcolor);
					char* _ = ia_strcat4(" style=\"background-color: ", bgt, "; color: ", fgt);
					char* style = ia_strcat(_, ";\"");
					free(_);
					TAG("span", style, 'R');
				} else if(escmsg[j] == 0x03) {
					j++;
					int k = j;
					for(; escmsg[j] != 0 && j < k + 2; j++) {
						if(!('0' <= escmsg[j] && escmsg[j] <= '9')) break;
					}
					if(escmsg[j] == ',') {
						j++;
						k = j;
						for(; escmsg[j] != 0 && j < k + 2; j++) {
							if(!('0' <= escmsg[j] && escmsg[j] <= '9')) break;
						}
					}
					j--;
				} else if(escmsg[j] == 0x0f) {
					int k;
					for(k = bufincr - 1; k >= 0; k--) {
						char c = buffer[k];
						if(c == 'B') {
							char* tmp = fmtmsg;
							fmtmsg = ia_strcat(tmp, "</b>");
							free(tmp);
						} else if(c == 'I') {
							char* tmp = fmtmsg;
							fmtmsg = ia_strcat(tmp, "</i>");
							free(tmp);
						} else if(c == 'U') {
							char* tmp = fmtmsg;
							fmtmsg = ia_strcat(tmp, "</u>");
							free(tmp);
						} else if(c == 'S') {
							char* tmp = fmtmsg;
							fmtmsg = ia_strcat(tmp, "</s>");
							free(tmp);
						} else if(c == 'R') {
							char* tmp = fmtmsg;
							fmtmsg = ia_strcat(tmp, "</span>");
							free(tmp);
						}
					}
					bufincr = 0;
					bgcolor = 0xd0d0ff;
					fgcolor = 0x000000;
				} else {
					cbuf[0] = escmsg[j];
					char* tmp;
					tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, cbuf);
					free(tmp);
				}
			}
			for(j = bufincr - 1; j >= 0; j--) {
				char c = buffer[j];
				if(c == 'B') {
					char* tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, "</b>");
					free(tmp);
				} else if(c == 'I') {
					char* tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, "</i>");
					free(tmp);
				} else if(c == 'U') {
					char* tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, "</u>");
					free(tmp);
				} else if(c == 'S') {
					char* tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, "</s>");
					free(tmp);
				} else if(c == 'R') {
					char* tmp = fmtmsg;
					fmtmsg = ia_strcat(tmp, "</span>");
					free(tmp);
				}
			}
			fprintf(f, "<span class=\"line\">[%s] &lt;%s&gt; %s</span>\n", date, escusr, fmtmsg);
			free(fmtmsg);
			free(escusr);
			free(escmsg);
			free(e[i]->username);
			free(e[i]->message);
			free(e[i]);
		}
		free(e);
		fprintf(f, "</code></pre>\n");
		fprintf(f, "		<div style=\"margin: 0; border: solid 1px black; width: 178px; min-height: 900px; padding: 10px; background-color: #d0d0ff; float: right; overflow: scroll\">Statistics<hr>%d messages</div>\n", i);
		fprintf(f, "		</div>\n");
		fprintf(f, "		<div style=\"clear: both\"></div>\n");
		fprintf(f, "		<hr>\n");
		fprintf(f, "		<a href=\"./\">Go back to index</a>\n");
		fprintf(f, "		<hr>\n");
		fprintf(f, "		<i>Generated by <a href=\"http://nishi.boats/ircarc\">IRC-Archiver</a> %s</i>\n", ircarc_version);
		fprintf(f, "	</body>\n");
		fprintf(f, "</html>\n");
		fclose(f);
		free(title);
	}
	free(path);
}
