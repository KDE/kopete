// aim.h
//
// Kit AIM client
//
// For copyright and license, see accompanying documentation

#ifndef AIM_H
#define AIM_H

#define TOC_PERMITALL 1
#define TOC_DENYALL 2
#define TOC_PERMITSOME 3
#define TOC_DENYSOME 4

#define TOC_USER_AOL 1
#define TOC_USER_ADMIN 2
#define TOC_USER_UNCONFIRMED 3
#define TOC_USER_NORMAL 4
#define TOC_USER_AWAY 5

#define TOC_HOST "toc.oscar.aol.com"
#define TOC_PORT 21
//#define TOC_PORT 9898
#define TOC_AUTH_HOST "login.oscar.aol.com"
#define TOC_AUTH_PORT "21"
//#define TOC_AUTH_PORT "5190"

#define TOC_LANG "english"

//#include "../config.h"

#define KIT_VER "Kinkatta 0.90"

//#define KIT_VER "\"kit client\"" KIT_VER_NUM

#define TOC_VER_INT 1

#include <qstring.h>

QString tocNormalize(const QString &);
QString tocRoast(const QString &);
QString tocProcess(const QString &);

#endif
