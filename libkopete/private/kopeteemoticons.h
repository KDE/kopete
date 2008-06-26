/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002-2003 by Stefan Gehn            <metz@gehn.net>
    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>
    Copyright (c) 2005      by Engin AYDOGAN	      <engin@bzzzt.biz>

	Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef kopeteemoticons_h__
#define kopeteemoticons_h__

#include "kopete_export.h"
#include <kemoticons.h>

class KEmoticons;
namespace Kopete {

class KOPETE_EXPORT Emoticons
{
public:
	/**
	 * The emoticons container-class by default is a singleton object.
	 * Use this method to retrieve the instance.
	 */
	static KEmoticons *self();

	static QString parseEmoticons(const QString &text, KEmoticonsTheme::ParseMode mode = KEmoticonsTheme::DefaultParse, const QStringList &exclude = QStringList());
	static QList<KEmoticonsTheme::Token> tokenize(const QString &message, KEmoticonsTheme::ParseMode mode = KEmoticonsTheme::DefaultParse);

};

} //END namespace Kopete

#endif
// vim: set noet ts=4 sts=4 sw=4:
