/*
    cryptographyplugin.h  -  description

    Copyright (c) 2002-2004 by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef CryptographyPLUGIN_H
#define CryptographyPLUGIN_H


#include "kopeteplugin.h"
//Added by qt3to4:
#include <QByteArray>
#include "cryptographyconfig.h"

class QStringList;
class QString;
class QTimer;

namespace Kopete
{
	class Message;
		class ChatSession;
	class SimpleMessageHandlerFactory;
}

/**
  * @author Olivier Goffart
  */

class CryptographyPlugin : public Kopete::Plugin
{
	Q_OBJECT

public:
	static CryptographyPlugin  *plugin();
	static QByteArray cachedPass();
	static void setCachedPass(const QByteArray &pass);
	static bool passphraseHandling();
	static const QRegExp isHTML;

	CryptographyPlugin( QObject *parent, const QStringList &args );
	~CryptographyPlugin();

public slots:

	void slotIncomingMessage( Kopete::Message& msg );
	void slotOutgoingMessage( Kopete::Message& msg );

private slots:

	void slotSelectContactKey();
	void slotForgetCachedPass();
	void loadSettings();
	
	void slotNewKMM(Kopete::ChatSession *);

private:
	static CryptographyPlugin* pluginStatic_;
	Kopete::SimpleMessageHandlerFactory *m_inboundHandler;
	QByteArray m_cachedPass;
	QTimer *m_cachedPass_timer;

	//cache messages for showing
	QMap<QString, QString> m_cachedMessages;

	//Settings
	QString mPrivateKeyID;
	unsigned int mCacheTime;
	bool mAlsoMyKey;
	bool mAskPassPhrase;
	CryptographyConfig::CacheMode mCachePassPhrase;
};

#endif

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

