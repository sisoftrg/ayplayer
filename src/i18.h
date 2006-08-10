/* AYplayer (c)2001-2006 sisoft//trg.
\* $Id: i18.h,v 1.3 2006/08/10 03:13:53 root Exp $ */
#ifndef __I18_H_
#define __I18_H_

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(String) gettext (String)
#define GT_INIT \
	setlocale(LC_ALL,""); \
	bindtextdomain(PACKAGE,LOCALEDIR); \
	textdomain(PACKAGE);
#else
#define _(String) String
#define GT_INIT
#endif

#endif
