/*
	abservicebinding.cpp: Address Book Service Binding

    Copyright (c) 2007		by Zhang Panyong	        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
namespace Papillon
{
/*******************************************************************
 * ABService Result Interal Class
 * ****************************************************************/
namespace Internal
{

FindABResult;;FindABResult()
{
}

FindABResult::~FindABResult()
{
	qDeleteAll(m_groups);
	qDeleteAll(m_contacts);
	qDeleteAll(m_dynamicItems);
}

FindABResult::setGroup(const QList<Group*> &Groups)
{
	m_groups = Groups;
}

QLis<Groups *> FindABResult::Groups()
{
	return m_groups
}

FindABResult::setContacts(const QList<Contact *> &contacts)
{
	m_contacts = contacts;
}

QList<Contact *> FindABResult::contacts()
{
	return m_contacts;
}

FindABResult::setDynamicItems(const QList <DynamicItem * > &dynamic_items)
{
	m_dynamicItems = dynamic_items;
}

QList<DynamicItem *> dynamicItems()
{
	return m_dynamicItems;
}

FindABResult::setCircleResult(CircleResult *circleResult)
{
	m_circleResult = circleResult;
}

FindABResult::circleResult()
{
	return m_circleResult;
}

FindABResult::setAB(const AddressBook ab)
{
	m_ab = ab;
}

AddressBook FindABResult::AB()
{
	return m_ab;
}

}//Internal
/*******************************************************************
 * ABService Class
 * ****************************************************************/

class ABServiceBinding::Private
{
public:
	Private()
	 : connection(0)
	{}

	QString	abLastChange;
	QString	gleamsLastChange;
	//= KDateTime::fromString(str , KDateTime::ISODate);
	//str::toString(KDateTime::ISODate);
	QString		server;
	HttpConnection *connection;
};

ABServiceBinding::ABServiceBinding(HttpConnection *connection, QObject *parent)
 : QObject(parent), d(new Private)
{
	d->server = QLatin1String("omega.contacts.msn.com");
	d->abLastChange = QString("0001-01-01T00:00:00.0000000-08:00");
	d->gleamsLastChange= QString("0001-01-01T00:00:00.0000000-08:00");
	d->connection = connection;
	connect(d->connection, SIGNAL(readyRead()), this, SLOT(connectionReadyRead()));
}

ABServiceBinding::~ABServiceBinding()
{
	delete d;
}

ABServiceBinding::setABLastChange(QString abLastChange)
{
	d->abLastChange = abLastChange;
}

ABServiceBinding::setGleamLastChange(QString gleamsLastChange)
{
	d->gleamsLastChange = gleamsLastChange;
}

void ABServiceBinding::connectToServer(const QString &server)
{
	QEventLoop tempEventLoop;
	connect(d->connection, SIGNAL(connected()), &tempEventLoop, SLOT(quit()));
	// FIXME: React on a connecting error.
	d->connection->connectToServer(server);

	tempEventLoop.exec();
}

void ABServiceBinding::findAddressBook()
{
	// Connect to server
	connectToServer(d->server);

	HttpTransfer *transfer = new HttpTransfer;
	transfer->setRequest( QLatin1String("POST"), QLatin1String("/abservice/abservice.asmx") );
	transfer->setContentType( QLatin1String("text/xml") );
	transfer->setValue( QLatin1String("Host"), server );
	transfer->setValue( QLatin1String("SOAPAction"), QLatin1String("http://www.msn.com/webservices/AddressBook/ABFindAll") );
	transfer->setValue( QLatin1String("User-Agent"), QLatin1String("libpapillon") );
	transfer->setValue( QLatin1String("Cookie"), d->connection->cookie() );

	QString abStr;
    if( d->abLastChange != QString("0001-01-01T00:00:00.0000000-08:00") )
	{
		abStr = QLatin1String( "<deltasOnly>true</deltasOnly><lastChange>%1</lastChange>").args(d->abLastChange);
	}
	else
	{
		abStr = QLatin1String("<deltasOnly>false</deltasOnly>")
	}

	QString gleamStr = QLatin1String("<dynamicItemView>Gleam</dynamicItemView>");
	if( d->gleamsLastChange != QString("0001-01-01T00:00:00.0000000-08:00") )
	{
		gleamStr = gleamStr + 
			QLatin1String( "<dynamicItemLastChange>%1</dynamicItemLastChange>").args(d->gleamsLastChange);
	}

	// TODO: Use Papillon Soap classes instead.
	QByteArray soapData = QString::fromUtf8("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soapenc=\"http://schemas.xmlsoap.org/soap/encoding/\">"
	"<soap:Header>"
		"<ABApplicationHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">"
			"<ApplicationId>09607671-1C32-421F-A6A6-CBFAA51AB5F4</ApplicationId>"
			"<IsMigration>false</IsMigration>"
			"<PartnerScenario>Initial</PartnerScenario>"
		"</ABApplicationHeader>"
		"<ABAuthHeader xmlns=\"http://www.msn.com/webservices/AddressBook\">"
			"<ManagedGroupRequest>false</ManagedGroupRequest>"
		"</ABAuthHeader>"
	"</soap:Header>"
	"<soap:Body>"
		"<ABFindAll xmlns=\"http://www.msn.com/webservices/AddressBook\">"
			"<abId>00000000-0000-0000-0000-000000000000</abId>"
			"<abView>Full</abView>"
			"%s"
		"</ABFindAll>"
	"</soap:Body>"
"</soap:Envelope>").args(abStr + gleamStr).toUtf8();

	transfer->setBody( soapData );

	d->connection->write(transfer);
}

void ABServiceBinding::connectionReadyRead()
{
	HttpTransfer *transfer = d->connection->read();
	if( !transfer )
	{
		//TODO:Error processing
	}

	// We longer need the HTTP connection, close it.
	d->connection->disconnectFromServer();

	QByteArray data = transfer->body();

	QDomDocument responseDocument;
	responseDocument.setContent(data, true);

	QDomNode node = responseDocument.documentElement().firstChild();
	while( !node.isNull() )
	{
		QDomElement bodyElement = node.toElement();
		if( (!bodyElement.isNull()) && ( bodyElement.tagName() == QLatin1String("Body") ) )
		{
			QDomElement abElement = bodyElement.firstChild().toElement();
			if( (!abElement.isNull()) && (abElement.tagName() == QLatin1String("ABFindAllResponse")) )
			{
				parseABResult( abElement );
			}

		}
		node = node.nextSibling();
	}
}

void ABServiceBinding::parseABResult(const QDomElement &allResponseElement)
{
	FindABResult *result = new FindABResult;
	QDomNode allResultNode = allResponseElement.firstChild();

	if(allResultNode.isNull())
	{
		return;
	}
	if(allResultNode.toElement().tagName() != QLatin1String("ABFindAllResult"))
	{
		return;
	}
	QDomNode resultNode = allResultNode.toElement().firstChild();

	while(!resultNode.isNull())
	{
		QDomElement resultElement = resultNode.toElement();
		if(resultElement.isNull())
		{
			break;
		}
		if(  resultElement.tagName() == QLatin1String("groups") )
		{
			QList<Group *> *groupList = parseGroups(resultElement);
			result->setGroups(groupList);
		}
		else if(  resultElement.tagName() == QLatin1String("contacts") )
		{
			QList<ABContact *> *abContactList = parseContacts(resultElement);
			result->setContacts(abContactList);
		}
		else if(  resultElement.tagName() == QLatin1String("DynamicItems") )
		{
			QList<DynamicItem *> *dynamicItemList = parseDynamicItems(resultElement);
			result->setDynamicItems( dynamicItemList );
		}
		else if(  resultElement.tagName() == QLatin1String("CircleResult") )
		{
			CircleResult *circleResult = parseCircleResult(resultElement);
			result->setCircleResult(circleResult);
		}
		else if(  resultElement.tagName() == QLatin1String("ab") )
		{
			AddressBook * addressbook = parseAB(resultElement);
			result->setAB( addressbook );
		}

		resultNode = resultNode.nextSibling();
	}

	emit findABResult(result);
}

Annotation * ABServiceBinding::parseAnnotation(const QDomElement &annotationElement)
{
	Annotation *annotation = new Annotation;
	QString name, value;
	QDomNode AnnNode = annotationElement.firstChild();
	while(!AnnNode.isNull())
	{
		QDomElement AnnElement = AnnNode.toElement();
		if(AnnElement.isNull())
		{
			break;
		}
		if(AnnElement.tagName() == QLatin1String("Name"))
		{
			name = AnnElement.text();
		}
		else if(AnnElement.tagName() == QLatin1String("Value"))
		{
			value = AnnElement.text();
			annotation->insert(name, value);
		}
		AnnNode = AnnNode.nextSibling();
	}
	return annotation;
}

QList<Annotation *> ABServiceBinding::parseAnnotations(const QDomElement &annotationElement)
{
	QList<Annotation *> annotationList;
	QDomNode childNode = annotationElement.firstChild();
	while(!childNode.isNull())
	{
		QDomElement childElement = childNode.toElement();
		if(childElement.isNull())
		{
			break;
		}
		if(childElement.tagName() == QLatin1String("annotations"))
		{
			QDomNode AnnNode = childElement.firstChild();
			while(!AnnNode.isNull())
			{
				QDomElement AnnElement = AnnNode.toElement();
				if(AnnElement.isNull())
				{
					break;
				}
				if(AnnElement.tagName() == QLatin1String("Annotation"))
				{
					Annotation * annotation = parseGroupAnnotation(AnnElement);
					annotationList << annotation;
				}
				AnnNode = AnnNode.nextSibling();
			}
		}
		else
		{
		}
		childNode = childNode.nextSibling();
	}
	return annotationList;
}

GroupInfo * ABServiceBinding::parseGroupInfo(const QDomElement &InfoElement)
{
	GroupInfo *groupInfo = new GroupInfo;
	QDomNode childNode = InfoElement.firstChild();
	while(!childNode.isNull())
	{
		QDomElement childElement = childNode.toElement();
		if(childElement.isNull())
		{
			break;
		}
		if(childElement.tagName() == QLatin1String("annotations"))
		{
			QList<Annotation *> annotationList = parseAnnotations(childElement);
			groupInfo->setAnnotation(annotationList);
		}
		else if(childElement.tagName() == QLatin1String("groupType"))
		{
			groupInfo->setType(childElement.text());
		}
		else if(childElement.tagName() == QLatin1String("name"))
		{
			groupInfo->setName(childElement.text());
		}
		else if(childElement.tagName() == QLatin1String("IsNotMobileVisible"))
		{
		}
		else if(childElement.tagName() == QLatin1String("IsPrivate"))
		{
		}
		childNode = childNode.nextSibling();
	}
	return groupInfo;
}

Group * ABServiceBinding::parseGroup(const QDomElement &groupElement)
{
	Group * newGroup = new Group;
	QDomNode InfoNode = groupElement.firstChild();
	while(!InfoNode.isNull())
	{
		QDomElement InfoElement = InfoNode.toElement();
		if(InfoElement.isNull())
		{
			break;
		}
		if( InfoElement.tagName() == QLatin1String("groupId") )
		{
			newgroup->setGroupId( InfoElement.text() );
		}
		else if( InfoElement.tagName() == QLatin1String("groupInfo") )
		{
			GroupInfo * groupInfo = parseGroupInfo(InfoElement);
			newGroup->setGroupInfo(groupInfo);
		}
		else if( InfoElement.tagName() == QLatin1String("propertiesChanged") )
		{
		}
		else if( InfoElement.tagName() == QLatin1String("fDeleted") )
		{
		}
		else if( InfoElement.tagName() == QLatin1String("lastChange") )
		{
			newgroup->setLastChange(InfoElement.text());
		}
		else
		{

		}
		InfoNode = InfoNode.nextSibling();
	}
	return newGroup;
}

QList<Group *> * ABServiceBinding::parseGroups(const QDomElement &groupsElement)
{
	QList<Group *> groupList;
	QDomNode groupNode = groupsElement.firstChild();
	while(!groupNode.isNull())
	{
		QDomElement groupElement = groupNode.toElement();
		if(groupElement.isNull())
		{
			break;
		}
		if(groupElement.tagName() == QLatin1String("Group"))
		{
			Group * group = parseGroup(groupElement);
			groupList << group;
		}
		groupNode = groupNode.nextSibling();
	}
	return groupList;
}

Locations * parseLocations(const QDomElement &locationsElement)
{
	Locations locations = new Locations;
	QDomNode locationNode = locationsElement.firstChild();

	while(!locationNode.isNull())
	{
		locationElement = locationNode.toElement();
		if(locationElement.tagName() != QLatin1String("ContactLocation"))
		{
			break;
		}
		QDomNode locNode = locationNode.firstChild();

		QString locType, city;
		while(!locNode.isNull())
		{
			if(locElement.tagName() == QLatin1String("contactLocationType"))
			{
				locType = locElement.text();
			}
			else if(locElement.tagName() == QLatin1String("city"))
			{
				city = locElement.text();
			}
			locNode = locNode.nextSibling();
		}
		locations->insert(locType, city);
		locationNode = locationNode.nextSibling();
	}
	return locations;
}

Phones * parsePhones(const QDomElement &phonesElement)
{
	Phones phones = new Phones;
	QDomNode phoneNode = phonesElement.firstChild();

	while(!phoneNode.isNull())
	{
		phoneElement = phoneNode.toElement();
		if(phoneElement.tagName() != QLatin1String("ContactPhone"))
		{
			break;
		}
		QDomNode phoNode = phoneNode.firstChild();

		QString phoType, number;
		while(!phoNode.isNull())
		{
			QDomElement = phoNode.toElement();
			if(phoElement.tagName() == QLatin1String("contactPhoneType"))
			{
				phoType = phoElement.text();
			}
			else if(phoElement.tagName() == QLatin1String("number"))
			{
				number = phoElement.text();
			}
			phoNode = phoNode.nextSibling();
		}
		phones->insert(phoType, number);
		phoneNode = phoneNode.nextSibling();
	}
	return phones;
}

Locations * parseEmails(const QDomElement &emailsElement)
{
	Emails emails = new Emails;
	QDomNode emailNode = emailsElement.firstChild();

	while(!emailNode.isNull())
	{
		emailElement = emailNode.toElement();
		if(emailElement.tagName() != QLatin1String("ContactEmail"))
		{
			break;
		}
		QDomNode emNode = emailNode.firstChild();

		QString emailType, email;
		while(!emNode.isNull())
		{
			if(emElement.tagName() == QLatin1String("contactEmailType"))
			{
				emailType = emElement.text();
			}
			else if(emElement.tagName() == QLatin1String("email"))
			{
				email = emElement.text();
			}
		}
		emails->insert(emType, email);
		emailNode = emailNode.nextSibling();
	}
	return emails;
}

ABContactInfo * ABServiceBinding::parseContactInfo(const QDomElement &contactInfoElement)
{
	ABContactInfo * abContactInfo = new ABContactInfo;
	QDomNode InfoNode = contactInfoElement.firstChild();
	QHash<QString,QString> m_contactInfo;
	while(!InfoNode.isNull())
	{
		QDomElement InfoElement = InfoNode.toElement();
		if(InfoElement.isNull())
		{
			break;
		}
		if(InfoElement.tagName() == QLatin1String("locations"))
		{
			Locations * locations = parseLocations(InfoElement);
			ABContactInfo->setLocations( locations );
		}
		else if(InfoElement.tagName() == QLatin1String("annotations"))
		{
			QList<Annotation *> annotationList = parseAnnotations(InfoElement);
			ABContactInfo->setAnnotations(annotationList);
		}
		else if(InfoElement.tagName() == QLatin1String("groupIds"))
		{
			QList<QString> gidList;
			gidNode = InfoElement.firstChild();
			while(!gidNode.isNull())
			{
				QDomElement gidElement = gidNode.toElement();
				if(gidElement.isNull())
				{
					break;
				}
				if(gidElement.tagName() == QLatin1String("gid"))
				{
					gidList << gidElement.text();
				}
				gidNode = gidNode.nextSibling();
			}
			ABContactInfo->setGroupIds(gidList);
		}
		else if(InfoElement.tagName() == QLatin1String("phones"))
		{
			Phones *phones = parsePhones(InfoElement);
			ABContactInfo->setPhones(phones);
		}
		else if(InfoElement.tagName() == QLatin1String("emails"))
		{
			Emails* emails = parseEmails(InfoElement);
			ABContactInfo->setEmails(emails);
		}
		else
		{
			QString idKey = InfoElement.tagName();
			QString data = InfoElement.text();
			m_contactInfo.insert(idKey, data);
	//		qDebug()<< "id:" << idKey<< " -- data:"<<data;
		}
		InfoNode = InfoNode.nextSibling();
	}
}

ABContact * ABServiceBinding::parseContact(const QDomElement &contactElement)
{
	ABContact *newAbContact = new ABContact;
	QDomNode InfoNode = contactElement.firstChild();
	while(!InfoNode.isNull())
	{
		QDomElement InfoElement = InfoNode.toElement();
		if(InfoElement.isNull())
		{
			break;
		}
		if(InfoElement.tagName() == QLatin1String("contactId"))
		{
			newAbContact->setcontactId(InfoElement.text());
		}
		else if(InfoElement.tagName() == QLatin1String("contactInfo"))
		{
			ABContactInfo * abContactInfo = parseContactInfo(InfoElement);
			newAbContact->setConactInfo(abContactInfo);
		}
		else if(InfoElement.tagName() == QLatin1String("propertiesChanged"))
		{
		}
		else if(InfoElement.tagName() == QLatin1String("fDeleted"))
		{
		}
		else if(InfoElement.tagName() == QLatin1String("lastChange"))
		{
			newAbContact->setLastChanged(InfoElement->text());
		}
		else 
		{
		}

		InfoNode = InfoNode.nextSibling();
	}
	return newAbContact;
}

QList<ABContact *>  *ABServiceBinding::parseContacts(const QDomElement &contactsElement)
{
	QList<ABContact *> contactList;
	QDomNode contactNode = contactsElement.firstChild();
	while(!contactNode.isNull())
	{
		QDomElement contactElement = contactNode.toElement();
		if(contactElement.isNull())
		{
			break;
		}
		if(contactElement.tagName() == QLatin1String("Contact"))
		{
			ABContact * abContact = parseContact(contactElement);
			contactList << abContact;
		}
		contactNode = contactNode.nextSibling();
	}
	return contactList;
}

DynamicItem * ABServiceBinding::parseDynamicItem(const QDomElement &dynamicItemElement)
{
	DynamicItem *dynamicItem = new DynamicItem;

	QHash <QString, QString> dynamicItemHash;
	QDomNode itemNode = dynamicItemElement.firstChild();
	while(!itemNode.isNull())
	{
		QDomElement itemElement = itemNode.toElement();
		if(itemElement.isNull())
		{
			break;
		}
		if(itemElement.tagName() == QLatin1String("Notifications"))
		{
		}
		else
		{
			QString idKey = itemElement.tagName();
			QString data = itemElement.text();
			m_dynamicItem.insert(idKey, data);
			qDebug()<< Q_FUNC_INFO << "Dynamic Item id:" << idKey<< " -- data:"<<data;
		}
		itemNode = itemNode.nextSibling();
	}
	dynamicItem->setItemHash(dynamicItemHash);

	return dynamicItem;
}

QList<DynamicItem *> *ABServiceBinding:;parseDynamicItems(const QDomElement &dynamicItemsElement)
{
	QList<DynamicItem *> *dynamicItemList;
	QDomNode dynamicItemNode = dynamicItemsElement.firstChild();

	while(!dynamicItemNode.isNull())
	{
		QDomElement dynamicItemElement = dynamicItemNode.toElement();
		if(dynamicItemElement.isNull())
		{
			break;
		}
		if(dynamicItemElement.tagName() == QLatin1String("DynamicItem"))
		{
			DynamicItem * dynamicItem = parseDynamicItem(dynamicItemElement);
			dynamicItemList << dynamicItem;
		}
		dynamicItemNode = dynamicItemNode.nextSibling();
	}
	return &dynamicItemList;
}

CircleResult * ABServiceBinding::parseCircleResult(const QDomElement &circleElement)
{
	CircleResult *circleResult = new CircleResult;
	QDomNode circleTicketNode = circleElement.firstChild();

	if(circleTicketNode.isNull())
	{
		return;
	}
	QDomElement circleTicketElement = circleTicketNode.toElement();
	if(circleTicketElement.tagName() == QLatin1String("CircleTicket"))
	{
		circleResult->setTicket(circleTicketElement.text());
	}
	return circleResult;
}

QHash<QString, QString> ABServiceBinding:;parseABInfo(const QDomElement &abInfosElement)
{
	QHash<QString, QString> m_abinfo;
	QDomNode abInfoNode = abInfosElement.firstChild();
	while(!abInfoNode.isNull())
	{
		QDomElement abInfoElement = abInfoNode.toElement();
		if(abInfoElement.isNull())
		{
			break;
		}
		else
		{
			QString idKey	= abInfoElement.tagName();
			QString data 	= abInfoElement.text();
			m_abinfo.insert(idKey, data);
			qDebug()<< Q_FUNC_INFO<< "AB Info id:" << idKey<< " - data:"<<data;
		}
		abInfoNode = abInfoNode.nextSibling();
	}
	return m_abinfo;
}

AddressBook *ABServiceBinding:;parseAB(const QDomElement &abElement)
{
	AddressBook *newAddressBook = new AddressBook;
	QDomNode abServiceNode = abElement.firstChild();

	while(!abServiceNode.isNull())
	{
		QDomElement abServiceElement = abServiceNode.toElement();
		if(abServiceElement.isNull())
		{
			break;
		}
		if(abServiceElement.tagName() == QLatin1String("abId"))
		{
			qDebug() << Q_FUNC_INFO << "abid"<<abServiceElement.text();
			newAddressBook->setabid(abServiceElement.text());
		}
		else if(abServiceElement.tagName() == QLatin1String("abInfo"))
		{
			AbInfo * abInfo = parseABInfo(abServiceElement);
			newAddressBook->setABInfo(abInfo);
		}
		else if(abServiceElement.tagName() == QLatin1String("lastChange"))
		{
			newAddressBook->setLastChange(abServiceElement.text());
		}
		else if(abServiceElement.tagName() == QLatin1String("DynamicItemLastChanged"))
		{
			newAddressBook->setDynamicItemLastChanged(abServiceElement.text());
		}
		else if(abServiceElement.tagName() == QLatin1String("createDate"))
		{
			newAddressBook->setCreateDate(abServiceElement.text());
		}
		else if(abServiceElement.tagName() == QLatin1String("propertiesChanged"))
		{
		}
		else
		{
		}
		abServiceNode = abServiceNode.nextSibling();
	}
	return newAddressBook;
}

}
#include "abservicebinding.moc"
