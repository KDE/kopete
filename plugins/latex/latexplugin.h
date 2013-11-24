/*
    latexplugin.h

    Kopete LaTeX Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>
    Copyright (c) 2004-2005 by Olivier Goffart  <ogoffart@kde. org>
    Copyright (c) 2009      by Fabian Rami      <fabian.rami@wowcompany.com>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef LATEXPLUGIN_H
#define LATEXPLUGIN_H

#include <QVariantList>
#include <qobject.h>
#include <qstring.h>

#include "kopetemessage.h"
#include "kopeteplugin.h"

class QStringList;
class QString;

class KTemporaryFile;

namespace Kopete { class Message; class ChatSession; }

/**
  * @author Duncan Mac-Vicar Prett
  */

class LatexPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static LatexPlugin  *plugin();

	LatexPlugin( QObject *parent, const QVariantList &args );
	~LatexPlugin();

public slots:
	void slotMessageAboutToShow( Kopete::Message& msg );
	void slotMessageAboutToSend( Kopete::Message& msg );
	void slotNewChatSession( Kopete::ChatSession *KMM);

public:
	/**
	 * gives a LaTeX formula, and return the filename of the file where the LaTeX is stored.
     */
	QString handleLatex(const QString &latex);

	/**
	 * return false if the LaTeX formula may contains malicious commands
	 */
	bool securityCheck(const QString & formula);

private:
	static LatexPlugin* s_pluginStatic;
	QString m_convScript;
	bool mMagickNotFoundShown;
	QList<KTemporaryFile *> m_tempFiles;
};

#endif
