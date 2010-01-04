/*
    kopeteinfoeventmanager.h - Kopete Info Event Manager

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETEINFOEVENTMANAGER_H
#define KOPETEINFOEVENTMANAGER_H

#include <QObject>
#include "kopete_export.h"

namespace Kopete {

class InfoEvent;

/**
 * The Info Event Manager that contains all info event items
 *
 * All info event items that are in Info Event Manager will be shown in
 * non-intrusive way to a user in Kopete Main Window.
 *
 * @author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT InfoEventManager : public QObject
{
Q_OBJECT
public:
	/**
	 * The Info Event Manager is a singleton class of which only a single
	 * instance will exist. If no manager exists yet this function will
	 * create one for you.
	 *
	 * @return the instance of the InfoEventManager
	 */
	static InfoEventManager *self();

	~InfoEventManager();

	/**
	 * Return all info events that are in the InfoEventManager.
	 */
	QList<InfoEvent*> events() const;

	/**
	 * Return number of info event items in the InfoEventManager.
	 */
	int eventCount() const;

	/**
	 * Return the info event at index position @p i in the InfoEventManager.
	 */
	Kopete::InfoEvent* event( int i ) const;

Q_SIGNALS:
	/**
	 * Emitted when the info event items in InfoEventManager has been changed.
	 */
	void changed();

	/**
	 * Emitted when new info event item is added into the InfoEventManager.
	 */
	void eventAdded( Kopete::InfoEvent* event );

	/**
	 * Emitted before new info event item is added into the InfoEventManager.
	 * Any class can connect to this signal and call @p close() on the @p event
	 * which will abort the adding.
	 *
	 * @param event the Info Event that will be added into InfoEventManager
	 */
	void eventAboutToBeAdded( Kopete::InfoEvent* event );

protected:
	friend class InfoEvent;
	/**
	 * Add info event
	 *
	 * @param event the Info Event that will be added into InfoEventManager
	 */
	void addEvent( Kopete::InfoEvent* event );

	using QObject::event;

private Q_SLOTS:
	void eventClosed( Kopete::InfoEvent* event );

private:
	InfoEventManager();

	static InfoEventManager *instance;

	class Private;
	Private * const d;
};

}

#endif
