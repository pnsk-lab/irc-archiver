/* $Id$ */

#ifndef __IA_DB_H__
#define __IA_DB_H__

int ia_db_init(void);
char* ia_escape_sql(const char* stuff);
int ia_db_put(const char* user, const char* channel, const char* message);

#endif
