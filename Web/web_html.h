/* $Id$ */

#ifndef __WEB_HTML_H__
#define __WEB_HTML_H__

#include "web_db.h"

char* web_html_escape(const char* html);
int web_html_generate(const char* name, web_range_t range);

#endif
