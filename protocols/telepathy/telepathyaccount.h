/*
 * This file is part of Kopete
 *
 * Copyright (C) 2009 Collabora Ltd. <http://www.collabora.co.uk/>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef TELEPATHYACCOUNT_H_
#define TELEPATHYACCOUNT_H_

#include <QObject>

#include <kopeteaccount.h>

#include <TelepathyQt4/Client/ConnectionManager>

namespace Kopete
{
    class MetaContact;
}

class TelepathyProtocol;

class TelepathyAccount : public Kopete::Account
{
    Q_OBJECT

public:
    TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId);
    ~TelepathyAccount();

	bool readConfig();
	QString connectionProtocol() const;
	Telepathy::Client::ProtocolParameterList allConnectionParameters() const;

public slots:
    virtual void connect (const Kopete::OnlineStatus &initialStatus = Kopete::OnlineStatus());
    virtual void disconnect ();
    virtual void setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(), const OnlineStatusOptions& options = None);
    virtual void setStatusMessage (const Kopete::StatusMessage &statusMessage);

protected:
    virtual bool createContact( const QString &contactId, Kopete::MetaContact *parentContact );

private:
	Telepathy::Client::ProtocolInfo *getProtocolInfo(QString protocol);
	Telepathy::Client::ConnectionManager *getConnectionManager();

	QString m_connectionManagerName;
	QString m_connectionProtocolName;
	Telepathy::Client::ProtocolParameterList m_connectionParameters;
	Telepathy::Client::ConnectionManager *m_connectionManager;
};

#endif // TELEPATHYACCOUNT_H_