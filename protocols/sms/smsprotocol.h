/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SMSPROTOCOL_H
#define SMSPROTOCOL_H

#include <qmap.h>
#include <qmovie.h>
#include <qpixmap.h>
#include <qptrdict.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kopeteprotocol.h"
#include "kopetecontact.h"

class KAction;
class KActionMenu;

class KopeteContact;
class KopeteMetaContact;
class KopeteMessage;
class KopeteMessageManager;
class SMSContact;

class SMSProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	SMSProtocol( QObject *parent, const char *name, const QStringList& args);
	~SMSProtocol();

	static SMSProtocol *protocol();

	bool unload();

	virtual void deserialize( KopeteMetaContact *metaContact,
		const QStringList &strList );

	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget( QWidget *parent );
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway();
	virtual void setAvailable();
	virtual bool isAway() const;

	SMSContact* addContact( const QString& nr , const QString& name, KopeteMetaContact *m=0L);

	KopeteContact *myself() const;

public slots:
	void serialize( KopeteMetaContact *metaContact);

private:
	static SMSProtocol* s_protocol;

	SMSContact *m_mySelf;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

