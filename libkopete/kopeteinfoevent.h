/*
    kopeteinfoevent.h - Kopete Info Event

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
#ifndef KOPETEINFOEVENT_H
#define KOPETEINFOEVENT_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>

#include "kopete_export.h"

namespace Kopete {

/**
 * Base class for all Info Events
 *
 * The info event will be shown in non-intrusive way
 * to user in Kopete Main Window.
 *
 * You have to use sendEvent to show the event.
 *
 * The pointer is automatically deleted when the event is closed.
 *
 *	@author Roman Jarosz <kedgedev@centrum.cz>
 */
class KOPETE_EXPORT InfoEvent : public QObject
{
Q_OBJECT
public:
	InfoEvent( QObject *parent = 0 );

	~InfoEvent();

	/**
	 * @return the Info Event title
	 */
	QString title() const;

	/**
	 * Set the Info Event title.
	 * @param title the title
	 */
	void setTitle( const QString& title );

	/**
	 * @return the Info Event text
	 */
	QString text() const;

	/**
	 * Set the Info Event text.
	 *
	 * The text is shown in a QLabel, you should make sure to escape any html that is needed.
	 * You can use some of the qt basic html tags.
	 *
	 * This text will also be shown in KNotification popup
	 *
	 * @param text the text
	 */
	void setText( const QString& text );

	/**
	 * @return the additional text
	 */
	QString additionalText() const;

	/**
	 * Set the additional text.
	 *
	 * This is only shown in InfoEditWidget
	 *
	 * @param text the additional text
	 */
	void setAdditionalText( const QString& text );

	/**
	 * @return the list of actions
	 */
	QMap<uint, QString> actions() const;

	/**
	 * Set the list of actions link.
	 * @param actions the list of actions
	 */
	void addAction( uint actionId, const QString& actionText );

	/**
	 * @return true if event should automatically be shown in contact list window
	 */
	bool showOnSend() const;

	/**
	 * Set if event should automatically be shown in contact list window.
	 * @param showOnSend the show flag
	 */
	void setShowOnSend( bool showOnSend );

	/**
	 * @return true if event has been closed and is scheduled for deletion.
	 */
	bool isClosed() const;

public Q_SLOTS:
	/**
	 * Emit the event.
	 */
	virtual void sendEvent();

	/**
	 * Activate the action specified action
	 */
	virtual void activate( uint actionId );

	/**
	 * Close the info event.
	 *
	 * This will delete the info event.
	 */
	void close();

Q_SIGNALS:
	/**
	 * User visible data has been changed.
	 */
	void changed();

	/**
	 * A action has been activated. This signal is only emitted if
	 * activate( uint ) is not replaced.
	 * @param actionId is the id of the activated action.
	 */
	void actionActivated( uint actionId );
	
	/**
	 * Emitted when the info event is closed.
	 */
	void eventClosed( Kopete::InfoEvent* event );

private:
	class Private;
	Private * const d;

};

}

#endif
