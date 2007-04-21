/*
   sharingservicebinding.cpp - Binding to MSN Sharing web service for contact list

   Copyright (c) 2007 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
 */
#include "sharingservicebinding.h"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtCore/QEventLoop>
#include <QtXml/QtXml>

// Papillon includes
#include "Papillon/Http/Connection"
#include "Papillon/Http/Transfer"

using namespace Papillon;

namespace Papillon
{

namespace Internal
{

//BEGIN FindMembershipResult
FindMembershipResult::~FindMembershipResult()
{
	qDeleteAll(m_services);
}

void FindMembershipResult::setServices(const QList<Service*> &service)
{
	m_services = service;
}

QList<Service*> FindMembershipResult::services() const
{
	return m_services;
}
//END FindMembershipResult

//BEGIN Service
Service::~Service()
{
	qDeleteAll(m_memberships);
}

void Service::setMemberships(const QList<Membership*> &memberships)
{
	m_memberships = memberships;
}

QList<Membership*> Service::memberships() const
{
	return m_memberships;
}

void Service::setLastChange(const QDateTime &lastChange)
{
	m_lastChange = lastChange;
}

QDateTime Service::lastChange() const
{
	return m_lastChange;
}
//END Service

//BEGIN Membership
Membership::~Membership()
{
	qDeleteAll(m_members);
}

void Membership::setMemberRole(const QString &memberRole)
{
	m_memberRole = memberRole;
}

QString Membership::memberRole() const
{
	return m_memberRole;
}

void Membership::setMembers(const QList<Member*> &members)
{
	m_members = members;
}

QList<Member*> Membership::members() const
{
	return m_members;
}

//END Membership

//BEGIN Member
void Member::setMembershipId(unsigned int value)
{
    m_membershipId = value;
}

unsigned int Member::membershipId() const
{
    return m_membershipId;
}

void Member::setType(const QString &value)
{
    m_type = value;
}

QString Member::type() const
{
    return m_type;
}

void Member::setDisplayName(const QString &value)
{
    m_displayName = value;
}

QString Member::displayName() const
{
    return m_displayName;
}

void Member::setState(const QString &value)
{
    m_state = value;
}

QString Member::state() const
{
    return m_state;
}

void Member::setDeleted(bool value)
{
    m_deleted = value;
}

bool Member::deleted() const
{
    return m_deleted;
}

void Member::setLastChanged(const QDateTime &value)
{
    m_lastChanged = value;
}

QDateTime Member::lastChanged() const
{
    return m_lastChanged;
}

void Member::setChanges(const QString &value)
{
    m_changes = value;
}

QString Member::changes() const
{
    return m_changes;
}

void Member::setPassportName(const QString &value)
{
    m_passportName = value;
}

QString Member::passportName() const
{
    return m_passportName;
}

void Member::setIsPassportNameHidden(bool value)
{
    m_isPassportNameHidden = value;
}

bool Member::isPassportNameHidden() const
{
    return m_isPassportNameHidden;
}

void Member::setPassportId(int value)
{
    m_passportId = value;
}

int Member::passportId() const
{
    return m_passportId;
}

void Member::setCID(int value)
{
    m_cID = value;
}

int Member::cID() const
{
    return m_cID;
}

void Member::setPassportChanges(const QString &value)
{
    m_passportChanges = value;
}

QString Member::passportChanges() const
{
    return m_passportChanges;
}

//END Member
class SharingServiceBinding::Private
{
public:
	Private()
	 : connection(0)
	{}

	HttpConnection *connection;
};

SharingServiceBinding::SharingServiceBinding(HttpConnection *connection, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->connection = connection;
	connect(d->connection, SIGNAL(readyRead()), this, SLOT(connectionReadyRead()));
}

SharingServiceBinding::~SharingServiceBinding()
{
	delete d;
}

void SharingServiceBinding::connectToServer(const QString &server)
{
	QEventLoop tempEventLoop;
	connect(d->connection, SIGNAL(connected()), &tempEventLoop, SLOT(quit()));
	// FIXME: React on a connecting error.
	d->connection->connectToServer(server);

	tempEventLoop.exec();
}

void SharingServiceBinding::findMembership()
{
	QString server = QLatin1String("omega.contacts.msn.com");

	// Connect to server
	connectToServer(server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/abservice/SharingService.asmx") );
	transfer->setContentType( QLatin1String("application/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.msn.com/webservices/AddressBook/FindMembership") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	// TODO: Use Papillon Soap classes instead.
QByteArray soapData = QString::fromUtf8("<?xml version='1.0' encoding='utf-8'?>\r\n"
"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
    "<soap:Header xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "<ABApplicationHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<ApplicationId xmlns=\"http://www.msn.com/webservices/AddressBook\">09607671-1C32-421F-A6A6-CBFAA51AB5F4</ApplicationId>\r\n"
            "<IsMigration xmlns=\"http://www.msn.com/webservices/AddressBook\">false</IsMigration>\r\n"
            "<PartnerScenario xmlns=\"http://www.msn.com/webservices/AddressBook\">Initial</PartnerScenario>\r\n"
        "</ABApplicationHeader>\r\n"
        "<ABAuthHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<ManagedGroupRequest xmlns=\"http://www.msn.com/webservices/AddressBook\">false</ManagedGroupRequest>\r\n"
        "</ABAuthHeader>\r\n"
    "</soap:Header>\r\n"
    "<soap:Body xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n"
        "<FindMembership xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
            "<serviceFilter xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
                "<Types xmlns=\"http://www.msn.com/webservices/AddressBook\">\r\n"
                    "<ServiceType xmlns=\"http://www.msn.com/webservices/AddressBook\">Messenger</ServiceType>\r\n"
                "</Types>\r\n"
            "</serviceFilter>\r\n"
        "</FindMembership>\r\n"
    "</soap:Body>\r\n"
"</soap:Envelope>\r\n").toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void SharingServiceBinding::connectionReadyRead()
{
	HttpTransfer *transfer = d->connection->read();
	if( transfer )
	{
		// We longer need the HTTP connection, close it.
		d->connection->disconnectFromServer();

		QByteArray data = transfer->body();

		QDomDocument responseDocument;
		responseDocument.setContent(data, true);

		QDomNode node = responseDocument.documentElement().firstChild();
		while( !node.isNull() )
		{
			QDomElement tempElement = node.toElement();
			if( !tempElement.isNull() )
			{
				if( tempElement.tagName() == QLatin1String("Body") )
				{
					if( !tempElement.firstChild().toElement().isNull() && tempElement.firstChild().toElement().tagName() == QLatin1String("FindMembershipResponse") )
					{
						parseFindMembershipResponse( tempElement.firstChild().toElement() );
					}
				}
			}

			node = node.nextSibling();
		}
		
	}
}

void SharingServiceBinding::parseFindMembershipResponse(const QDomElement &element)
{
	FindMembershipResult *result = new FindMembershipResult;
	// Got empty response
	if( !element.firstChild().isNull() )
	{
		// Services
		QDomNode nodeResult = element.firstChild().firstChild();
		while( !nodeResult.isNull() )
		{
			QDomElement resultElement = nodeResult.toElement();
			if( !resultElement.isNull() )
			{
				if( resultElement.tagName() == QLatin1String("Services") )
				{
					QDomNode serviceNode = resultElement.firstChild();
					QList<Service*> serviceList;
					for(; !serviceNode.isNull(); serviceNode = serviceNode.nextSibling())
					{
						if( serviceNode.isElement() )
						{
							Service *newService = parseService(serviceNode.toElement());
							serviceList << newService;
						}
					}
					result->setServices( serviceList );
				}
			}
			nodeResult = nodeResult.nextSibling();
		}
	}

	emit findMembershipResult(result);
}

Service* SharingServiceBinding::parseService(const QDomElement &element)
{
	Service *newService = new Service;
	QDomNode node = element.firstChild();
	while( !node.isNull() )
	{
		QDomElement tempElement = node.toElement();
		if( !tempElement.isNull() )
		{
			if( tempElement.tagName() == QLatin1String("Memberships") )
			{
				QList<Membership*> memberships;
				QDomNode nodeMemberships = tempElement.firstChild();
				for(; !nodeMemberships.isNull(); nodeMemberships = nodeMemberships.nextSibling())
				{
					if( nodeMemberships.isElement() )
					{
						Membership *newMembership = parseMembership( nodeMemberships.toElement() );
						memberships << newMembership;
					}
				}
				newService->setMemberships( memberships );
			}
			if( tempElement.tagName() == QLatin1String("LastChange") )
			{
				QDateTime lastChange = QDateTime::fromString(tempElement.text(), Qt::ISODate);
				newService->setLastChange(lastChange);
			}
			if( tempElement.tagName() == QLatin1String("Info") )
			{
			}
		}

		node = node.nextSibling();
	}

	return newService;
}

Membership *SharingServiceBinding::parseMembership(const QDomElement &membership)
{
	Membership *newMembership = new Membership;
	
	QDomNode node = membership.firstChild();
	while( !node.isNull() )
	{
		QDomElement tempElement = node.toElement();
		
		if( !tempElement.isNull() )
		{
			if( tempElement.tagName() == QLatin1String("MemberRole") )
			{
				newMembership->setMemberRole( tempElement.text() );
			}
			if( tempElement.tagName() == QLatin1String("Members") )
			{
				QList<Member*> members;
				QDomNode memberNode = tempElement.firstChild();
				for(; !memberNode.isNull(); memberNode = memberNode.nextSibling())
				{
					if( memberNode.isElement() )
					{
						Member *newMember = parseMember(memberNode.toElement());
						members << newMember;
					}
				}
				newMembership->setMembers(members);
			}
		}

		node = node.nextSibling();
	}

	return newMembership;
}

Member *SharingServiceBinding::parseMember(const QDomElement &member)
{
	Member *newMember = new Member;

	QDomNode node = member.firstChild();
	while( !node.isNull() )
	{
		QDomElement tempElement = node.toElement();
		if( !tempElement.isNull() )
		{
			if( tempElement.tagName() == QLatin1String("MembershipId") )
			{
				newMember->setMembershipId( tempElement.text().toUInt() );
			}
			if( tempElement.tagName() == QLatin1String("Type") )
			{
				newMember->setType( tempElement.text() );
			}
			if( tempElement.tagName() == QLatin1String("State") )
			{
				newMember->setState( tempElement.text() );
			}
			if( tempElement.tagName() == QLatin1String("Deleted") )
			{
				bool value = false;
				QString text = tempElement.text().toLower();
				if( text == QLatin1String("true") )
				{
					value = true;
				}
				else if( text == QLatin1String("false") )
				{
					value = false;
				}
				newMember->setDeleted(value);
			}
			if( tempElement.tagName() == QLatin1String("LastChanged") )
			{
				QDateTime value = QDateTime::fromString(tempElement.text(), Qt::ISODate);
				newMember->setLastChanged(value);
			}
			if( tempElement.tagName() == QLatin1String("Changes") )
			{
				newMember->setChanges( tempElement.text() );
			}
			if( tempElement.tagName() == QLatin1String("PassportName") )
			{
				newMember->setPassportName( tempElement.text() );
			}
			if( tempElement.tagName() == QLatin1String("IsPassportNameHidden") )
			{
				bool value = false;
				QString text = tempElement.text().toLower();
				if( text == QLatin1String("true") )
				{
					value = true;
				}
				else if( text == QLatin1String("false") )
				{
					value = false;
				}
				newMember->setIsPassportNameHidden(value);
			}
			if( tempElement.tagName() == QLatin1String("PassportId") )
			{
				newMember->setPassportId( tempElement.text().toInt() );
			}
			if( tempElement.tagName() == QLatin1String("CID") )
			{
				newMember->setCID( tempElement.text().toInt() );
			}
			if( tempElement.tagName() == QLatin1String("PassportChanges") )
			{
				newMember->setPassportChanges( tempElement.text() );
			}
		}

		node = node.nextSibling();
	}

	return newMember;
}

} // Internal

} // Papillon

#include "sharingservicebinding.moc"
