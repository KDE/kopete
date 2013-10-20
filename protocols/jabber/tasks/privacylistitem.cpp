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

#include <kdebug.h>
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
		act = "Deny";
	else 
		act = "Allow";
	
	QString what;
	if (all()) 
		what = "All";
	else {
		if (message()) 
			what += "Messages,";
		if (presenceIn()) 
			what += "Presence-In,";
		if (presenceOut()) 
			what += "Presence-Out,";
		if (iq()) 
			what += "Queries,";
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
	QDomElement item = doc.createElement("item");
	
	if (type_ == JidType) 
		item.setAttribute("type","jid");
	else if (type_ == GroupType)
		item.setAttribute("type","group");
	else if (type_ == SubscriptionType)
		item.setAttribute("type","subscription");
	
	if (type_ != FallthroughType)
		item.setAttribute("value",value_);
	
	if (action_ == Allow) 
		item.setAttribute("action","allow");
	else
		item.setAttribute("action","deny");

	item.setAttribute("order", order_);
	
	if (!(message_ && presenceIn_ && presenceOut_ && iq_)) {
		if (message_)
			item.appendChild(doc.createElement("message"));
		if (presenceIn_)
			item.appendChild(doc.createElement("presence-in"));
		if (presenceOut_)
			item.appendChild(doc.createElement("presence-out"));
		if (iq_)
			item.appendChild(doc.createElement("iq"));
	}

	return item;
}

void PrivacyListItem::fromXml(const QDomElement& el)
{
	//kDebug (JABBER_DEBUG_GLOBAL) << "Parsing privacy list item";
	if (el.isNull() || el.tagName() != "item") {
		kWarning (JABBER_DEBUG_GLOBAL) << "Invalid root tag for privacy list item.";
		return;
	}

	QString type = el.attribute("type"); 
	if (type == "jid")
		type_ = JidType;
	else if (type == "group")
		type_ = GroupType;
	else if (type == "subscription")
		type_ = SubscriptionType;
	else
		type_ = FallthroughType;
	
	QString value = el.attribute("value");
	value_ = value;
	if (type_ == JidType && XMPP::Jid(value_).isEmpty()) 
		kWarning (JABBER_DEBUG_GLOBAL) << "Invalid value for item of type 'jid'.";
	else if (type_ == GroupType && value_.isEmpty()) 
		kWarning (JABBER_DEBUG_GLOBAL) << "Empty value for item of type 'group'.";
	else if (type_ == SubscriptionType && value_ != "from" && value != "to" && value_ != "both" && value_ != "none")
		kWarning (JABBER_DEBUG_GLOBAL) << "Invalid value for item of type 'subscription'.";
	else if (type_ == FallthroughType && !value_.isEmpty()) 
		kWarning (JABBER_DEBUG_GLOBAL) << "Value given for item of fallthrough type.";
		
	QString action = el.attribute("action");
	if (action == "allow") 
		action_ = Allow;
	else if (action == "deny")
		action_ = Deny;
	else
		kWarning (JABBER_DEBUG_GLOBAL) << "Invalid action given for item.";

	bool ok;
	order_ = el.attribute("order").toUInt(&ok);
	if (!ok)
		kWarning (JABBER_DEBUG_GLOBAL) << "Invalid order value for item.";

	if (el.hasChildNodes()) {
		message_ = !el.firstChildElement("message").isNull();
		presenceIn_ = !el.firstChildElement("presence-in").isNull();
		presenceOut_ = !el.firstChildElement("presence-out").isNull();
		iq_ = !el.firstChildElement("iq").isNull();
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
