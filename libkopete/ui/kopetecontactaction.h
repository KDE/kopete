/*
    kopetecontactaction.cpp - KAction for selecting a Kopete::Contact

    Copyright (c) 2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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

namespace Kopete
{
class Contact;
}

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KOPETE_EXPORT KopeteContactAction : public KAction
{
	Q_OBJECT

public:
	/**
	 * Create a new KopeteContactAction
	 */
	KopeteContactAction( Kopete::Contact *contact, const QObject* receiver, const char* slot, KAction* parent );
	~KopeteContactAction();

	Kopete::Contact * contact() const;

signals:
	/**
	 * Overloaded signal to get the selected contact
	 */
	void activated( Kopete::Contact *action );

private slots:
	void slotContactActionActivated();

private:
	Kopete::Contact *m_contact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

