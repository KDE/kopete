/*
 * privacylistitem.cpp
 * Copyright (C) 2006  Remko Troncon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include "privacylistitem.h"
#include <QDomElement>
#include <QObject>

#include "xmpp_xmlcommon.h"
#include "xmpp_jid.h"

#include "jabber_protocol_debug.h"
#include "jabberprotocol.h"

PrivacyListItem::PrivacyListItem() : message_(true), presenceIn_(true), presenceOut_(true), iq_(true)
{ 
}

PrivacyListItem::PrivacyListItem(const QDomElement& e) 
{
	fromXml(e);
}

bool PrivacyListItem::isBlock() const
{
	return (type() == JidType && action() == Deny && all());
}
	
QString PrivacyListItem::toString() const
{
	QString act;
	if (action() == PrivacyListItem::Deny) 
		act = QStringLiteral("Deny");
	else 
		act = QStringLiteral("Allow");
	
	QString what;
	if (all()) 
		what = QStringLiteral("All");
	else {
		if (message()) 
			what += QLatin1String("Messages,");
		if (presenceIn()) 
			what += QLatin1String("Presence-In,");
		if (presenceOut()) 
			what += QLatin1String("Presence-Out,");
		if (iq()) 
			what += QLatin1String("Queries,");
		what.truncate(what.length()-1);
	}
	
	QString txt;
	if (type() == PrivacyListItem::FallthroughType) {
		txt = QString(QObject::tr("Else %1 %2")).arg(act).arg(what);
	}
	else {
		if (type() == PrivacyListItem::JidType) {
			txt = QString(QObject::tr("If JID is '%1' then %2 %3")).arg(value()).arg(act).arg(what);
		}
		else if (type() == PrivacyListItem::GroupType) {
			txt = QString(QObject::tr("If Group is '%1' then %2 %3")).arg(value()).arg(act).arg(what);
		}
		else if (type() == PrivacyListItem::SubscriptionType) {
			txt = QString(QObject::tr("If Subscription is '%1' then %2 %3")).arg(value()).arg(act).arg(what);
		}
	}

	return txt;
}

QDomElement PrivacyListItem::toXml(QDomDocument& doc) const
{
	QDomElement item = doc.createElement(QStringLiteral("item"));
	
	if (type_ == JidType) 
		item.setAttribute(QStringLiteral("type"),QStringLiteral("jid"));
	else if (type_ == GroupType)
		item.setAttribute(QStringLiteral("type"),QStringLiteral("group"));
	else if (type_ == SubscriptionType)
		item.setAttribute(QStringLiteral("type"),QStringLiteral("subscription"));
	
	if (type_ != FallthroughType)
		item.setAttribute(QStringLiteral("value"),value_);
	
	if (action_ == Allow) 
		item.setAttribute(QStringLiteral("action"),QStringLiteral("allow"));
	else
		item.setAttribute(QStringLiteral("action"),QStringLiteral("deny"));

	item.setAttribute(QStringLiteral("order"), order_);
	
	if (!(message_ && presenceIn_ && presenceOut_ && iq_)) {
		if (message_)
			item.appendChild(doc.createElement(QStringLiteral("message")));
		if (presenceIn_)
			item.appendChild(doc.createElement(QStringLiteral("presence-in")));
		if (presenceOut_)
			item.appendChild(doc.createElement(QStringLiteral("presence-out")));
		if (iq_)
			item.appendChild(doc.createElement(QStringLiteral("iq")));
	}

	return item;
}

void PrivacyListItem::fromXml(const QDomElement& el)
{
	//qDebug (JABBER_PROTOCOL_LOG) << "Parsing privacy list item";
	if (el.isNull() || el.tagName() != QLatin1String("item")) {
		qCWarning (JABBER_PROTOCOL_LOG) << "Invalid root tag for privacy list item.";
		return;
	}

	QString type = el.attribute(QStringLiteral("type")); 
	if (type == QLatin1String("jid"))
		type_ = JidType;
	else if (type == QLatin1String("group"))
		type_ = GroupType;
	else if (type == QLatin1String("subscription"))
		type_ = SubscriptionType;
	else
		type_ = FallthroughType;
	
	QString value = el.attribute(QStringLiteral("value"));
	value_ = value;
	if (type_ == JidType && XMPP::Jid(value_).isEmpty()) 
		qCWarning (JABBER_PROTOCOL_LOG) << "Invalid value for item of type 'jid'.";
	else if (type_ == GroupType && value_.isEmpty()) 
		qCWarning (JABBER_PROTOCOL_LOG) << "Empty value for item of type 'group'.";
	else if (type_ == SubscriptionType && value_ != QLatin1String("from") && value != QLatin1String("to") && value_ != QLatin1String("both") && value_ != QLatin1String("none"))
		qCWarning (JABBER_PROTOCOL_LOG) << "Invalid value for item of type 'subscription'.";
	else if (type_ == FallthroughType && !value_.isEmpty()) 
		qCWarning (JABBER_PROTOCOL_LOG) << "Value given for item of fallthrough type.";
		
	QString action = el.attribute(QStringLiteral("action"));
	if (action == QLatin1String("allow")) 
		action_ = Allow;
	else if (action == QLatin1String("deny"))
		action_ = Deny;
	else
		qCWarning (JABBER_PROTOCOL_LOG) << "Invalid action given for item.";

	bool ok;
	order_ = el.attribute(QStringLiteral("order")).toUInt(&ok);
	if (!ok)
		qCWarning (JABBER_PROTOCOL_LOG) << "Invalid order value for item.";

	if (el.hasChildNodes()) {
		message_ = !el.firstChildElement(QStringLiteral("message")).isNull();
		presenceIn_ = !el.firstChildElement(QStringLiteral("presence-in")).isNull();
		presenceOut_ = !el.firstChildElement(QStringLiteral("presence-out")).isNull();
		iq_ = !el.firstChildElement(QStringLiteral("iq")).isNull();
	}
	else {
		message_ = presenceIn_ = presenceOut_ = iq_ = true;
	}
}

PrivacyListItem PrivacyListItem::blockItem(const QString& jid)
{
	PrivacyListItem it;
	it.setType(JidType);
	it.setAction(Deny);
	it.setAll();
	it.setValue(jid);
	return it;
}
