/*
    kopetecontactaction.cpp - KAction for selecting a Kopete::Contact

    Copyright (c) 2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2006 by Matt Rogers            <mattr@kde.org>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetecontactaction_h__
#define __kopetecontactaction_h__

#include <kaction.h>
#include "kopete_export.h"

class KActionCollection;
namespace Kopete
{
class Contact;

namespace UI
{

/**
 * @author Martijn Klingens <klingens@kde.org>
 * @author Matt Rogers <mattr@kde.org>
 */
class KOPETE_EXPORT ContactAction : public KAction
{
	Q_OBJECT
public:
	/**
	 * Create a new KopeteContactAction
	 *
	 * The icon, text, and internal KAction name are taken from the
	 * Kopete::Contact object given to this constructor
	 *
	 * @param contact the contact this action is for
	 * @param parent the collection this action belongs to. The action is automatically added to the collection.
	 *
	 */
	ContactAction( Kopete::Contact *contact, KActionCollection* parent );

signals:
	/**
	 * Emitted when the action is triggered. Connect to this slot when
	 * you need to know which contact the action was triggered for
	 */
	void triggered( Kopete::Contact*, bool checked );
	void triggered( const QString &, bool checked );

private slots:
	/**
	 * @internal
	 * Reimplemented to emit triggered with a Kopete::Contact object
	 */
	void slotTriggered( bool );

private:
	Kopete::Contact *m_contact;
};

}

}
#endif
// vim: set noet ts=4 sts=4 sw=4:
// kate: tab-width 4; auto-insert-doxygen on; indent-mode cstyle;

