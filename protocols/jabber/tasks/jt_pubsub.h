 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JT_PUBSUB_H
#define JT_PUBSUB_H

// XEP-0060: Publish-Subscribe

#include "xmpp_task.h"
#include "xmpp_jid.h"
#include "xmpp_pubsubitem.h"
#include <QDomElement>

class QString;
//class PubSubItem;

class JT_PubSubPublish : public XMPP::Task
{
	Q_OBJECT
public:
	JT_PubSubPublish(XMPP::Task *parent, const QString &node, XMPP::PubSubItem &psitem);
	~JT_PubSubPublish();

	void onGo();
	bool take(const QDomElement &);

private:
	QDomElement mIQ;
};

#endif
