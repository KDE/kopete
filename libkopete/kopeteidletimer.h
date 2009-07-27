/*
    kopeteidletimer.h  -  Kopete Idle Timer

    Copyright (c) 2002      by Hendrik vom Lehn      <hvl@linux-4-ever.de>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETEIDLETIMER_H
#define KOPETEIDLETIMER_H

#include <QtCore/QObject>

#include "kopete_export.h"

namespace Kopete
{

/**
 * IdleTimer handles global idle time and allows to register idle timeout notifications
 *
 * IdleTimer is a singleton, you may uses it with @ref IdleTimer::self()
 */
class KOPETE_EXPORT IdleTimer : public QObject
{
Q_OBJECT

public:
	/**
	 * Get the only instance of IdleTimer
	 * @return IdleTimer single instance
	 */
	static IdleTimer *self();

	~IdleTimer();

	/**
	 * @brief Time in seconds the user has been idle
	 */
	int idleTime();

public Q_SLOTS:
	/**
	 * @brief Register new timeout notification
	 * \param idleSeconds the idle notification time period
	 * \param receiver the object that receives the timeout notification.
	 * \param memberActive the slot that is called when user has changed its state from idle to active.
	 * \param memberIdle the slot that is called when user was idle for @param idleSeconds seconds.
	 */
	void registerTimeout( int idleSeconds, QObject * receiver,
	                      const char * memberActive, const char * memberIdle );

	/**
	 * removes all registered timeout notifications for this object
	 */
	void unregisterTimeout( QObject *receiver );

private slots:
	void updateIdleTime();

private:
	IdleTimer();

	static IdleTimer *instance;

	class Private;
	Private * const d;
};

}

#endif
