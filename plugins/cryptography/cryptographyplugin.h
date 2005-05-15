/***************************************************************************
                          cryptographyplugin.h  -  description
                             -------------------
    begin                : jeu nov 14 2002
    copyright            : (C) 2002-2004 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CryptographyPLUGIN_H
#define CryptographyPLUGIN_H


#include "kopeteplugin.h"

class QStringList;
class QString;
class QTimer;

namespace Kopete
{
	class Message;
	class MetaContact;
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
	enum CacheMode
	{
		Keep	= 0,
		Time	= 1,
		Never	= 2
	};

	static CryptographyPlugin  *plugin();
	static QCString cachedPass();
	static void setCachedPass(const QCString &pass);
	static bool passphraseHandling();
	static const QRegExp isHTML;

	CryptographyPlugin( QObject *parent, const char *name, const QStringList &args );
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
	QCString m_cachedPass;
	QTimer *m_cachedPass_timer;

	//cache messages for showing
	QMap<QString, QString> m_cachedMessages;

	//Settings
	QString mPrivateKeyID;
	int mCacheMode;
	unsigned int mCacheTime;
	bool mAlsoMyKey;
	bool mAskPassPhrase;
	bool mCachePassPhrase;
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

