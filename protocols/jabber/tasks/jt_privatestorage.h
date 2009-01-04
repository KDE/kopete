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

#ifndef JT_PRIVATESTORAGE_H
#define JT_PRIVATESTORAGE_H

#include "xmpp_task.h"
#include "xmpp_jid.h"

class QDomElement;
class QString;

    
class JT_PrivateStorage : public XMPP::Task
{
	Q_OBJECT
	public:
		JT_PrivateStorage(XMPP::Task *parent);
		~JT_PrivateStorage();

		void set(const QDomElement &);
		void get(const QString &tag, const QString& xmlns);
		
		QDomElement element();

		void onGo();
		bool take(const QDomElement &);
		
	private:
		class Private;
		Private * const d;
};

#endif
