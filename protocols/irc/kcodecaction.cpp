/*
    kcodecaction.cpp

    Copyright (c) 2005      by Tommi Rantala  <tommi.rantala@cs.helsinki.fi>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qstringlist.h>
#include <qtextcodec.h>
#include <kcharsets.h>

#include "kcodecaction.h"

KCodecAction::KCodecAction( const QString &text, const KShortcut &cut,
		QObject *parent, const char *name ) : KSelectAction( text, "", cut, parent, name )
{
	QObject::connect( this, SIGNAL( activated( const QString & ) ),
			this, SLOT( slotActivated( const QString & ) ) );

	setItems( KCodecAction::supportedEncodings() );
}

void KCodecAction::slotActivated( const QString & text )
{
	/* text is something like "Western European ( iso-8859-1 )", but we must give
	 * codecForName() only the "iso-8859-1" part.
	 */
	QString encoding = KGlobal::charsets()->encodingForName(text);

	emit activated( KGlobal::charsets()->codecForName(encoding) );
}

void KCodecAction::setCodec( const QTextCodec *codec )
{
	QStringList items = this->items();
	int i = 0;
	for (QStringList::ConstIterator it = items.begin(), end = items.end(); it != end; ++it, ++i) {
		QString encoding = KGlobal::charsets()->encodingForName(*it);

		if (KGlobal::charsets()->codecForName(encoding)->mibEnum() == codec->mibEnum()) {
			setCurrentItem(i);
			break;
		}
	}
}

/* Create a list of supported encodings, and keep only one of each encoding
 * mime name.
 *
 * This piece of code from kdepim/kmail/kmmsgbase.cpp
 */

QStringList KCodecAction::supportedEncodings(bool usAscii)
{
	QStringList encodingNames = KGlobal::charsets()->availableEncodingNames();
	QStringList encodings;
	QMap<QString, bool> mimeNames;

	for (QStringList::ConstIterator it = encodingNames.begin();
			it != encodingNames.end(); ++it)
	{
		QTextCodec *codec = KGlobal::charsets()->codecForName(*it);
		QString mimeName = (codec) ? QString(codec->mimeName()).lower() : (*it);
		if (mimeNames.find(mimeName) == mimeNames.end())
		{
			encodings.append(KGlobal::charsets()->languageForEncoding(*it)
					+ " ( " + mimeName + " )");
			mimeNames.insert(mimeName, true);
		}
	}

	encodings.sort();
	if (usAscii) encodings.prepend(KGlobal::charsets()
			->languageForEncoding("us-ascii") + " ( us-ascii )");
	return encodings;
}

#include "kcodecaction.moc"
