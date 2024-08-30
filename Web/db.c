/* $Id$ */

#include "web_db.h"

#include "ia_db.h"
#include "ia_util.h"

#include <stdio.h>
#include <stdlib.h>

#include <sqlite3.h>

extern sqlite3* sql;

entry_t** retarr;

int add_info(void* param, int ncol, char** row, char** col) {
	entry_t* entry = malloc(sizeof(*entry));
	entry->username = ia_strdup(row[0]);
	entry->message = ia_strdup(row[2]);
	entry->time = strtoull(row[3], (char**)NULL, 10);

	entry_t** oldret = retarr;
	int i;
	for(i = 0; oldret[i] != NULL; i++)
		;
	retarr = malloc(sizeof(*retarr) * (i + 2));
	for(i = 0; oldret[i] != NULL; i++) {
		retarr[i] = oldret[i];
	}
	retarr[i] = entry;
	retarr[i + 1] = NULL;
	free(oldret);

	return 0;
}

int compare_time(const void* _a, const void* _b) {
	entry_t** a = (entry_t**)_a;
	entry_t** b = (entry_t**)_b;
	return (*a)->time - (*b)->time;
}

entry_t** web_db_query(web_range_t range) {
	char date[512];
	char* esc = ia_escape_sql(range.channel);
	char* query = ia_strcat3("select * from log where channel = '", esc, "'");

	retarr = malloc(sizeof(*retarr));
	retarr[0] = NULL;

	if(range.from > 0) {
		char* tmp = query;
		sprintf(date, "%llu", range.from);
		query = ia_strcat3(tmp, " and date >= ", date);
		free(tmp);
	}
	if(range.to > 0) {
		char* tmp = query;
		sprintf(date, "%llu", range.to);
		query = ia_strcat3(tmp, " and date <= ", date);
		free(tmp);
	}

	char* err;
	int ret = sqlite3_exec(sql, query, add_info, NULL, &err);
	if(ret != SQLITE_OK) {
		sqlite3_free(err);
	}

	free(query);

	int i;
	for(i = 0; retarr[i] != NULL; i++)
		;

	qsort(retarr, i, sizeof(entry_t*), compare_time);

	return retarr;
}
