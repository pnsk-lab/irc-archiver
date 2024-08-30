/* $Id$ */

#include "ia_util.h"

#include <stdlib.h>
#include <string.h>

char* ia_strcat(const char* a, const char* b) {
	char* str = malloc(strlen(a) + strlen(b) + 1);
	memcpy(str, a, strlen(a));
	memcpy(str + strlen(a), b, strlen(b));
	str[strlen(a) + strlen(b)] = 0;
	return str;
}

char* ia_strcat3(const char* a, const char* b, const char* c) {
	char* tmp = ia_strcat(a, b);
	char* str = ia_strcat(tmp, c);
	free(tmp);
	return str;
}

char* ia_strcat4(const char* a, const char* b, const char* c, const char* d) {
	char* tmp = ia_strcat3(a, b, c);
	char* str = ia_strcat(tmp, d);
	free(tmp);
	return str;
}

char* ia_strdup(const char* str) { return ia_strcat(str, ""); }
