/*
    history2logger.cpp

    Copyright (c) 2012 by Volker Härtel <cyberbeat@gmx.de>
    Copyright (c) 2003-2004 by Olivier Goffart        <ogoffart@kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "history2logger.h"

#include <QtCore/QRegExp>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QList>
#include <QtCore/QDate>
#include <QtGui/QTextDocument>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QHash>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <ksavefile.h>

#include "kopeteglobal.h"
#include "kopetecontact.h"
#include "kopeteprotocol.h"
#include "kopeteaccount.h"
#include "kopetemetacontact.h"
#include "kopetemessage.h"
#include "kopetechatsession.h"
#include "kopetecontactlist.h"

#include "history2dialog.h"
#include "history2config.h"


History2Logger* History2Logger::m_Instance = 0;

History2Logger::History2Logger( ) {

	QString path = KStandardDirs::locateLocal ( "appdata", "kopete_history.db" );
	m_db = QSqlDatabase::addDatabase ( "QSQLITE", "kopete-history" );
	m_db.setDatabaseName(path);
	if ( !m_db.open()) {
		return;
	}

	// Creates the tables if they do not exist.
	QSqlQuery query( "SELECT name FROM sqlite_master WHERE type='table'", m_db );
	query.exec();

	QStringList result;
	while (query.next()) {
		result.append(query.value(0).toString());
	}
	if ( !result.contains ( "history" ) ) {
		query.exec(QString ( "CREATE TABLE history "
		                     "(id INTEGER PRIMARY KEY,"
							 "protocol TEXT,"
							 "account TEXT,"
		                     "direction TEXT,"
		                     "me_id TEXT,"
		                     "me_nick TEXT,"
		                     "other_id TEXT,"
		                     "other_nick TEXT,"
		                     "datetime TEXT,"
		                     "message TEXT"
		                     ")" ));

		query.exec( QString ( "CREATE INDEX datetime ON history (datetime)"));
		query.exec( QString ( "CREATE INDEX contact ON history (protocol, account, other_id, datetime)"));
	}
}

History2Logger::~History2Logger() {
	m_db.close();
}

void History2Logger::beginTransaction(){
	QSqlQuery query("BEGIN TRANSACTION",m_db);
	query.exec();
}

void History2Logger::commitTransaction(){
	QSqlQuery query("COMMIT TRANSACTION",m_db);
	query.exec();
}

void History2Logger::appendMessage( const Kopete::Message &msg , const Kopete::Contact *ct, bool skipDuplicate ) {
	if(!msg.from())
		return;
	if (!msg.timestamp().isValid())
		return;
	// If no contact are given: If the manager is availiable, use the manager's
	// first contact (the channel on irc, or the other contact for others protocols
	const Kopete::Contact *c = ct;
	if(!c && msg.manager() ) {
		QList<Kopete::Contact*> mb=msg.manager()->members() ;
		c = mb.first();
	}
	if(!c) { //If the contact is still not initialized, use the message author.
		c =   msg.direction()==Kopete::Message::Outbound ? msg.to().first() : msg.from()  ;
	}

	const Kopete::Contact *me;
	const Kopete::Contact *other;
	if (msg.direction() == msg.Inbound) {
		me = msg.to().first();
		other = msg.from();
	} else if (msg.direction() == msg.Outbound) {
		me = msg.from();
		other = msg.to().first();
	} else {
		return;
	}

	QSqlQuery query(m_db);

	if (skipDuplicate){
		if (messageExists(msg, c)){
			return;
		}
	}


	query.prepare("INSERT INTO history (direction, protocol, account, me_id, me_nick, other_id, other_nick, datetime, message) "
	              "VALUES (:direction, :protocol, :account, :me_id, :me_nick, :other_id, :other_nick, :datetime, :message)");
	query.bindValue(":direction", msg.direction());
	query.bindValue(":me_id",me->contactId() );
	query.bindValue(":me_nick", me->displayName());
	query.bindValue(":other_id",other->contactId() );
	query.bindValue(":other_nick", other->displayName());
	query.bindValue(":datetime",msg.timestamp());
	query.bindValue(":protocol", ct->protocol()->pluginId());
	query.bindValue(":account", ct->account()->accountId());
	query.bindValue(":message", msg.plainBody());
	query.exec();
}

bool History2Logger::messageExists( const Kopete::Message &msg , const Kopete::Contact *ct) {
	if(!msg.from())
		return true;

	// If no contact are given: If the manager is availiable, use the manager's
	// first contact (the channel on irc, or the other contact for others protocols
	const Kopete::Contact *c = ct;
	if(!c && msg.manager() ) {
		QList<Kopete::Contact*> mb=msg.manager()->members() ;
		c = mb.first();
	}
	if(!c) { //If the contact is still not initialized, use the message author.
		c =   msg.direction()==Kopete::Message::Outbound ? msg.to().first() : msg.from()  ;
	}

	const Kopete::Contact *me;
	const Kopete::Contact *other;
	if (msg.direction() == msg.Inbound) {
		me = msg.to().first();
		other = msg.from();
	} else if (msg.direction() == msg.Outbound) {
		me = msg.from();
		other = msg.to().first();
	} else {
		return true;
	}

	QSqlQuery query(m_db);

	query.prepare("SELECT 1 FROM history WHERE direction = :direction AND protocol = :protocol AND account= :account AND me_id = :me_id AND other_id = :other_id AND datetime = :datetime AND message = :message");

	query.bindValue(":direction", msg.direction());
	query.bindValue(":me_id",me->contactId() );
	query.bindValue(":other_id",other->contactId() );
	query.bindValue(":datetime",msg.timestamp());
	query.bindValue(":protocol", ct->protocol()->pluginId());
	query.bindValue(":account", ct->account()->accountId());
	query.bindValue(":message", msg.plainBody());
	query.exec();
	if (query.next()){
		return true;
	}
	return false;
}


QList<Kopete::Message> History2Logger::readMessages(QDate date, const Kopete::MetaContact *c) {
	QList<Kopete::Message> messages;
	Kopete::Account *account;
	Kopete::Contact *from;
	Kopete::Contact *to;
	Kopete::Contact *other;
	Kopete::Contact *me;

	QStringList list;
	foreach (Kopete::Contact *ct, c->contacts()) {
		list.append("(other_id = '"+ct->contactId()+"' AND protocol = '"+ct->account()->protocol()->pluginId()+"' AND account = '"+ct->account()->accountId()+"')");
	}
	QSqlQuery query("SELECT * FROM history WHERE ("+list.join(" OR ") + ") AND datetime LIKE '"+date.toString(Qt::ISODate)+"%' ORDER BY datetime",m_db);
	query.exec();
	while (query.next()) {
		QSqlRecord r = query.record();
		other = 0;
		foreach (Kopete::Contact *ct, c->contacts()) {
			if (ct->contactId() == r.value("other_id").toString()) {
				other = ct;
			}
		}
		if (!other)
			continue;
		account = other->account();
		me = account->myself();
		if (r.value("direction").toString() == "0") {
			from = other;
			to = me;
		} else {
			from = me;
			to = other;
		}
		Kopete::Message m(from, to);
		m.setDirection(r.value("direction").toString() == "0"?Kopete::Message::Inbound:Kopete::Message::Outbound);
		QString message(r.value("message").toString());

		m.setHtmlBody(message);

		m.setTimestamp(r.value("datetime").toDateTime());
		messages.append(m);
	}
	return messages;
}

QList<Kopete::Message> History2Logger::readMessages(int lines, int offset,
        const Kopete::MetaContact *c, bool reverseOrder) {

	QList<Kopete::Message> messages;
	Kopete::Account *account;
	Kopete::Contact *from;
	Kopete::Contact *to;
	Kopete::Contact *other;
	Kopete::Contact *me;

	QStringList list;
	foreach (Kopete::Contact *ct, c->contacts()) {
		list.append("(other_id = '"+ct->contactId()+"' AND protocol = '"+ct->account()->protocol()->pluginId()+"' AND account = '"+ct->account()->accountId()+"')");
	}
	QString queryString = "SELECT * FROM history WHERE ("+list.join(" OR ") + ") ORDER BY datetime";
	if (reverseOrder)
		queryString += " DESC";
	queryString += QString(" LIMIT %1 OFFSET %2").arg(lines).arg(offset);

	QSqlQuery query(queryString, m_db);
	query.exec();
	while (query.next()) {
		QSqlRecord r = query.record();
		other = 0;
		foreach (Kopete::Contact *ct, c->contacts()) {
			if (ct->contactId() == r.value("other_id").toString()) {
				other = ct;
			}
		}
		if (!other)
			continue;
		account = other->account();
		me = account->myself();
		if (r.value("direction").toString() == "0") {
			from = other;
			to = me;
		} else {
			from = me;
			to = other;
		}
		Kopete::Message m(from, to);
		m.setDirection(r.value("direction").toString() == "0"?Kopete::Message::Inbound:Kopete::Message::Outbound);
		QString message(r.value("message").toString());
		m.setHtmlBody(message);
		m.setTimestamp(r.value("datetime").toDateTime());
		if (reverseOrder)
			messages.prepend(m);
		else
			messages.append(m);
	}
	return messages;
}


QList<QDate> History2Logger::getDays(const Kopete::MetaContact *c, QString search) {
	QList<QDate> dayList;
	QString queryString;
	QString searchQuery = "";
	if (!search.isEmpty())
		searchQuery = " AND message LIKE '%"+search+"%'";

	QStringList list;
	foreach (Kopete::Contact *ct, c->contacts()) {
		list.append("(other_id = '"+ct->contactId()+"' AND protocol = '"+ct->account()->protocol()->pluginId()+"' AND account = '"+ct->account()->accountId()+"')");
	}
	queryString = "SELECT DISTINCT strftime('%Y-%m-%d',datetime) AS day FROM history WHERE ("+list.join(" OR ") + ")  "+searchQuery+" ORDER BY datetime";

	QSqlQuery query(queryString, m_db);
	query.exec();
	while (query.next()) {
		dayList.append( query.value(0).toDate());
	}

	return dayList;
}

QList<DMPair> History2Logger::getDays(QString search) {
	QList<DMPair> dayList;
	QHash<QString, QHash< Kopete::MetaContact*, int>* > hash;
	QList<QString> dates;
	QString queryString;
	QString searchQuery = "";
	if (!search.isEmpty())
		searchQuery = "WHERE message LIKE '%"+search+"%'";
	queryString = "SELECT DISTINCT strftime('%Y-%m-%d',datetime) AS day, protocol, account, other_id FROM history "+searchQuery+" ORDER BY datetime";

	QSqlQuery query(queryString, m_db);
	query.exec();
	while (query.next()) {
		Kopete::Contact *c = Kopete::ContactList::self()->findContact(query.value(1).toString(),query.value(2).toString(),query.value(3).toString());
		if (!c)
			continue;
		QString date = query.value(0).toString();
		if (!hash.contains(date)){
			hash.insert(date, new QHash< Kopete::MetaContact*, int>());
			dates.append(date);
		}
		QHash< Kopete::MetaContact*, int> *h = hash.value(date);
		h->insert(c->metaContact(),1);
	}
	foreach (QString date, dates){
		foreach (Kopete::MetaContact *mc, hash.value(date)->keys()){
			DMPair pair(QDate::fromString(date, Qt::ISODate), mc);
			dayList.append(pair);
		}
	}

	return dayList;
}
#include "history2logger.moc"
