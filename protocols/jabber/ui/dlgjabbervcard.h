
/***************************************************************************
                          dlgjabbervcard.h  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGJABBERVCARD_H
#define DLGJABBERVCARD_H

#include <qdom.h>
#include "xmpp_types.h"
#include "xmpp_vcard.h"
#include "dlgvcard.h"

class QDomElement;
class QDomDocument;

class dlgJabberVCard:public dlgVCard
{
  Q_OBJECT public:
	  dlgJabberVCard (QWidget * parent = 0, const char *name = 0, Jabber::JT_VCard * vCard = 0);
	 ~dlgJabberVCard ();

	void assignVCard (Jabber::JT_VCard * vCard);

	public slots:void slotClose ();
	void slotSaveNickname ();
	void setReadOnly (bool);

	  signals:void updateNickname (const QString &);
	void saveAsXML (QDomElement &);

  private:
	  bool mIsReadOnly;
	QDomDocument doc;
	QDomElement textTag (const QString &, const QString &);
};

#endif // DLGJABBERVCARD_H

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
