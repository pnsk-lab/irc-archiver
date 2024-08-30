/* $Id$ */

#include "ia_db.h"

#include "ia_util.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <sqlite3.h>

sqlite3* sql;

extern char* database;

int ia_db_init(void) {
	int ret;
	ret = sqlite3_open(database, &sql);
	if(ret != SQLITE_OK) {
		return 1;
	}
	ret = sqlite3_exec(sql, "create table if not exists log(user text, channel text, message text, date number)", NULL, NULL, NULL);
	if(ret != SQLITE_OK) {
		return 1;
	}
	return 0;
}

char* escape_sql(const char* stuff) {
	char* str = malloc(strlen(stuff) * 2 + 1);
	int incr = 0;
	int i;
	for(i = 0; stuff[i] != 0; i++) {
		if(stuff[i] == '\'') {
			str[incr++] = '\'';
			str[incr++] = '\'';
		} else {
			str[incr++] = stuff[i];
		}
	}
	str[incr] = 0;
	return str;
}

int ia_db_put(const char* user, const char* channel, const char* message) {
	char* eusr = escape_sql(user);
	char* echn = escape_sql(channel);
	char* emsg = escape_sql(message);

	char* date = malloc(512);
	sprintf(date, "%llu", (unsigned long long)time(NULL));

	char* tmp;
	char* str;
	tmp = ia_strcat("insert into log values('", eusr);
	str = ia_strcat(tmp, "', '");
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, echn);
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, "', '");
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, emsg);
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, "', ");
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, date);
	free(tmp);

	tmp = str;
	str = ia_strcat(tmp, ")");
	free(tmp);

	free(date);
	free(eusr);
	free(echn);
	free(emsg);

	int ret = sqlite3_exec(sql, str, NULL, NULL, NULL);

	return ret == SQLITE_OK ? 0 : 1;
}
