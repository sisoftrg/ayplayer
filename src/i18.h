//(c)2004 sisoft\trg - AYplayer.
/* $Id: i18.h,v 1.1 2004/03/11 17:27:58 root Exp $ */
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