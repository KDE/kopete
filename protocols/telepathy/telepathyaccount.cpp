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

#include "telepathyaccount.h"
#include "telepathyprotocol.h"
#include "telepathycontact.h"

#include <kopetemetacontact.h>
#include <kopetecontactlist.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <ksharedconfig.h>

TelepathyAccount::TelepathyAccount(TelepathyProtocol *protocol, const QString &accountId)
    : Kopete::Account(protocol, accountId), m_connectionManager(0)
{
    kDebug(TELEPATHY_DEBUG_AREA);

    setMyself( new TelepathyContact(this, accountId, Kopete::ContactList::self()->myself()) );
}

TelepathyAccount::~TelepathyAccount()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::connect (const Kopete::OnlineStatus &initialStatus)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::disconnect ()
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setOnlineStatus (const Kopete::OnlineStatus &status, const Kopete::StatusMessage &reason, const OnlineStatusOptions& options)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

void TelepathyAccount::setStatusMessage (const Kopete::StatusMessage &statusMessage)
{
    kDebug(TELEPATHY_DEBUG_AREA);
}

bool TelepathyAccount::createContact( const QString &contactId, Kopete::MetaContact *parentContact )
{
    kDebug(TELEPATHY_DEBUG_AREA);
    return false;
}

bool TelepathyAccount::readConfig()
{
    kDebug(TELEPATHY_DEBUG_AREA);

    // Restore config not related to ConnectionManager parameters first
    // so that the UI for the protocol parameters will be generated
    KConfigGroup *accountConfig = configGroup();
    m_connectionManagerName = accountConfig->readEntry( QLatin1String("ConnectionManager"), QString() );
	// setup CM early
	getConnectionManager();
	
    m_connectionProtocolName = accountConfig->readEntry( QLatin1String("SelectedProtocol"), QString() );

    // Clear current connection parameters
    m_connectionParameters.clear();

    // Get the preferences from the connection manager to get the right types
    Telepathy::Client::ProtocolInfo* protocolInfo = getProtocolInfo(m_connectionProtocolName);
    if(!protocolInfo)
    {
        kDebug(TELEPATHY_DEBUG_AREA) << "Error: could not get protocol info" << m_connectionProtocolName;
        return false;
    }

    Telepathy::Client::ProtocolParameterList tempParameters = protocolInfo->parameters();

    // Now update the preferences
    KSharedConfig::Ptr telepathyConfig = KGlobal::config();
    QMap<QString,QString> allEntries = telepathyConfig->entryMap( TelepathyProtocol::protocol()->formatTelepathyConfigGroup(m_connectionManagerName, m_connectionProtocolName, accountId()));
    QMap<QString,QString>::ConstIterator it, itEnd = allEntries.constEnd();
    for(it = allEntries.constBegin(); it != itEnd; ++it)
    {
        foreach(Telepathy::Client::ProtocolParameter *parameter, tempParameters)
        {
            if( parameter->name() == it.key() )
            {
                kDebug(TELEPATHY_DEBUG_AREA) << parameter->defaultValue().toString() << it.value();
                if( parameter->defaultValue().toString() != it.value() )
                {
                    QVariant oldValue = parameter->defaultValue();
                    QVariant newValue(oldValue.type());
                    if ( oldValue.type() == QVariant::String )
                        newValue = QVariant(it.value());
                    else if( oldValue.type() == QVariant::Int )
                        newValue = QVariant(it.value()).toInt();
                    else if( oldValue.type() == QVariant::UInt )
                        newValue = QVariant(it.value()).toUInt();
                    else if( oldValue.type() == QVariant::Double )
                        newValue = QVariant(it.value()).toDouble();
                    else if( oldValue.type() == QVariant::Bool)
                    {
                        if( it.value().toLower() == "true")
                            newValue = true;
                        else
                            newValue = false;
                    }
                    else
                        newValue = QVariant(it.value());

                    kDebug(TELEPATHY_DEBUG_AREA) << "Name: " << parameter->name() << " Value: " << newValue << "Type: " << parameter->defaultValue().typeName();
                    m_connectionParameters.append(
                        new Telepathy::Client::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            newValue,
                            Telepathy::ConnMgrParamFlagHasDefault
                        ));
                }
                else
                {
                    kDebug(TELEPATHY_DEBUG_AREA) << parameter->name() << parameter->defaultValue();

                    m_connectionParameters.append(
                        new Telepathy::Client::ProtocolParameter(
                            parameter->name(),
                            parameter->dbusSignature(),
                            parameter->defaultValue(),
                            Telepathy::ConnMgrParamFlagHasDefault
                        ));
                }
            }
        }
    }

    kDebug(TELEPATHY_DEBUG_AREA) << m_connectionManagerName << m_connectionProtocolName << m_connectionParameters;
    if( !m_connectionManagerName.isEmpty() &&
        !m_connectionProtocolName.isEmpty() &&
        !m_connectionParameters.isEmpty() )
            return true;
    else
        return false;
}

QString TelepathyAccount::connectionProtocol() const
{
    kDebug(TELEPATHY_DEBUG_AREA);
	return m_connectionProtocolName;
}

Telepathy::Client::ProtocolParameterList TelepathyAccount::allConnectionParameters() const
{
    kDebug(TELEPATHY_DEBUG_AREA);
	return m_connectionParameters;
}

Telepathy::Client::ConnectionManager *TelepathyAccount::getConnectionManager()
{
    kDebug(TELEPATHY_DEBUG_AREA);
	if(!m_connectionManager)
	{
		m_connectionManager = new Telepathy::Client::ConnectionManager(m_connectionManagerName);
		// dont need wait until operation finished
		m_connectionManager->becomeReady();
	}
	
	return m_connectionManager;
}

Telepathy::Client::ProtocolInfo *TelepathyAccount::getProtocolInfo(QString protocol)
{
	Telepathy::Client::ProtocolInfoList protocolList = getConnectionManager()->protocols();

    foreach(Telepathy::Client::ProtocolInfo* protocolInfo, protocolList)
    {
		if(protocolInfo->name() == protocol)
		{
			kDebug(TELEPATHY_DEBUG_AREA) << protocolInfo->name();
	        return protocolInfo;
		}
    }
	return NULL;
}
