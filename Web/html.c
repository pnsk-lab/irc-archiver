/* $Id$ */

#include "web_html.h"

#include "ia_util.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* webroot;

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

int web_html_generate(const char* name, web_range_t range) {
	char* path = ia_strcat4(webroot, "/", name, ".html");
	FILE* f = fopen(path, "w");
	if(f != NULL) {
		char* htmlesc = web_html_escape(name);
		char* title = ia_strcat("Archive: ", htmlesc);
		fprintf(f, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n");
		fprintf(f, "<html>\n");
		fprintf(f, "	<head>\n");
		fprintf(f, "		<meta http-equiv=\"Content-Type\" content=\"text/html;charset=UTF-8\">\n");
		fprintf(f, "		<title>%s</title>\n", title);
		fprintf(f, "	</head>\n");
		fprintf(f, "	<body>\n");
		fprintf(f, "	</body>\n");
		fprintf(f, "</html>\n");
		fclose(f);
		free(title);
		free(htmlesc);
	}
	free(path);
}
