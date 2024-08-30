/* $Id$ */

#ifndef __WEB_DB_H__
#define __WEB_DB_H__

typedef struct {
	unsigned long long from;
	unsigned long long to;
	const char* channel;
} web_range_t;

typedef struct {
	char* username;
	char* message;
	unsigned long long time;
} entry_t;

entry_t** web_db_query(web_range_t range);

#endif
