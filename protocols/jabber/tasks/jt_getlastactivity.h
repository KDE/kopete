 /*
    Copyright (c) 2007      by Olivier Goffart  <ogoffart@kde.org>

    Kopete    (c) 2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JT_GETLASTACTIVITY_H
#define JT_GETLASTACTIVITY_H


#include "xmpp_task.h"	
#include "xmpp_jid.h"

#include <QDomElement>

class QString;

class JT_GetLastActivity : public XMPP::Task
{
	Q_OBJECT
	public:
		JT_GetLastActivity(XMPP::Task *);
		~JT_GetLastActivity();

		void get(const XMPP::Jid &);

		int seconds() const;
		const QString &message() const;

		void onGo();
		bool take(const QDomElement &x);

	private:
		class Private;
		Private * const d;

		QDomElement iq;
		XMPP::Jid jid;
};

#endif

