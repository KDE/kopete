/*
    kopetecontactaction.cpp - KAction for selecting a KopeteContact

    Copyright (c) 2003 by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef __kopetecontactaction_h__
#define __kopetecontactaction_h__

#include <kaction.h>

class KopeteContact;

/**
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopeteContactAction : public KAction
{
	Q_OBJECT

public:
	/**
	 * Create a new KopeteContactAction
	 */
	KopeteContactAction( KopeteContact *contact, const QObject* receiver, const char* slot, KAction* parent );
	~KopeteContactAction();

	KopeteContact * contact() const;

signals:
	/**
	 * Overloaded signal to get the selected contact
	 */
	void activated( KopeteContact *action );

private slots:
	void slotContactActionActivated();

private:
	KopeteContact *m_contact;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

