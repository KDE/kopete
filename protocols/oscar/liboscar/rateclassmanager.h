/*
    Kopete Oscar Protocol
    rateclassmanager.h - Manages the rates we get from the OSCAR server

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef RATECLASSMANAGER_H
#define RATECLASSMANAGER_H

#include <qobject.h>
#include <QList>
#include <qmap.h>
#include "oscartypes.h"

class Transfer;
class SnacTransfer;
class RateClass;
class Connection;
class RateClassManagerPrivate;


class RateClassManager : public QObject
{
Q_OBJECT
public:
	RateClassManager( Connection* parent );
	~RateClassManager();

	/** Reset the rate manager */
	void reset();

	/** Tell the rate manager about the new class */
	void registerClass( RateClass* );

	//! Check if we can send the packet right away
	bool canSend( Transfer* t ) const;

	//! Queue a transfer for sending later
	void queue( Transfer* t );

	/** Get the list of rate classes */
	QList<RateClass*> classList() const;
	
	/** Recalculate the rate levels for all the classes */
	void recalcRateLevels();

	/**
	 * Find the rate class for the snac and
	 * calculate time until we get to initial level
	 * \return the time in milliseconds that we need to wait
	 */
	int timeToInitialLevel( Oscar::SNAC s );

public slots:

	void transferReady( Transfer* );

private:

	/** Find the rate class for the transfer */
	RateClass* findRateClass( SnacTransfer* st ) const;

private:

	RateClassManagerPrivate* d;

};

#endif

//kate: tab-width 4; indent-mode csands;
