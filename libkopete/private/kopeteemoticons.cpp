/*
    kopeteemoticons.cpp - Kopete Preferences Container-Class

    Copyright (c) 2002      by Stefan Gehn            <metz@gehn.net>
    Copyright (c) 2002-2006 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2005      by Engin AYDOGAN          <engin@bzzzt.biz>

   Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteemoticons.h"
#include "kopeteappearancesettings.h"
/*
 * Testcases can be found in the kopeteemoticontest app in the tests/ directory.
 */


namespace Kopete {

K_GLOBAL_STATIC(KEmoticons, s_self)

KEmoticons *Emoticons::self()
{
	return s_self;
}

QString Emoticons::parseEmoticons(const QString &text, KEmoticonsTheme::ParseMode mode, const QStringList &exclude)
{
	if ( Kopete::AppearanceSettings::self()->useEmoticons() )
	{
		return Kopete::Emoticons::self()->theme().parseEmoticons(text, mode, exclude);
	} else
	{
		return text;
	}
}
QList<KEmoticonsTheme::Token> Emoticons::tokenize(const QString &message, KEmoticonsTheme::ParseMode mode)
{
	if ( Kopete::AppearanceSettings::self()->useEmoticons() )
	{
		QList<KEmoticonsTheme::Token> ret = Kopete::Emoticons::self()->theme().tokenize(message, mode);

		if( !ret.size() )
		{
			ret.append( KEmoticonsTheme::Token( KEmoticonsTheme::Text, message ) );
		}

		return ret;
	} else
	{
		QList<KEmoticonsTheme::Token> result;
		result.append( KEmoticonsTheme::Token( KEmoticonsTheme::Text, message ) );
		return result;
	}
}

} //END namesapce Kopete

// vim: set noet ts=4 sts=4 sw=4:
