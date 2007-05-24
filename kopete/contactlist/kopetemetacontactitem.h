/*
    kopetemetacontactlvi.h - Kopete Meta Contact K3ListViewItem

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>
    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002      by Duncan Mac-Vicar P     <duncan@kde.org>
    Copyright (c) 2007      by Matt Rogers            <mattr@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMETACONTACTLVI_H
#define KOPETEMETACONTACTLVI_H

#include <QtGui/QStandardItem>
#include <qobject.h>
#include <qpixmap.h>
#include <q3ptrdict.h>

class QVariant;

class KAction;
class KSelectAction;

namespace Kopete
{
class Account;
class MetaContact;
class Contact;
class Group;
class MessageEvent;
}

class AddContactPage;
class KopeteGroupItem;


/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Matt Rogers <mattr@kde.org>
 */
class KopeteMetaContactItem : public QObject, QStandardItem
{
	Q_OBJECT

public:
	explicit KopeteMetaContactItem( Kopete::MetaContact *contact );
	~KopeteMetaContactItem();

	/**
	 * metacontact this visual item represents
	 */
	Kopete::MetaContact *metaContact() const;

private:
	Kopete::MetaContact *m_metaContact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

