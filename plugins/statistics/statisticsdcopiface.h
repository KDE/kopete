/*
    statisticsdcopiface.h

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef STATISTICSDCOP_H
#define STATISTICSDCOP_H

#include <dcopobject.h>


class StatisticsDCOPIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
	/**
	 * Shows the statistics dialog for contact which has KABC id \var contactId
	 */
	virtual void dcopStatisticsDialog(QString contactId) = 0;
	/**
     * \returns true if contact was online at time timeStamp, false else. Returns false if contact does not exist.
	 */
	virtual bool dcopWasOnline(QString id, int timeStamp) = 0;
	/**
	 * \returns true if contact was online at dt, false else. Returns false if contact does not exist or if date is invalid.
	 */
	virtual bool dcopWasOnline(QString id, QString datetime) = 0;
	
	/**
	 * \returns true if contact was away at time timeStamp, false else. Returns false if contact does not exist.
	 */
	virtual bool dcopWasAway(QString id, int timeStamp) = 0;
	/**
	 * \returns true if contact was away at dt, false else. Returns false if contact does not exist or if date is invalid.
	 */
	virtual bool dcopWasAway(QString id, QString datetime) = 0;
	
	/**
	 * \returns true if contact was offline at time timeStamp, false else. Returns false if contact does not exist.
	 */
	virtual bool dcopWasOffline(QString id, int timeStamp) = 0;
	/**
	 * \returns true if contact was offline at dt, false else. Returns false if contact does not exist or if date is invalid.
	 */
	virtual bool dcopWasOffline(QString id, QString datetime) = 0;
	
	/**
	 * \returns return the status of the contact at datetime.
	 */
	virtual QString dcopStatus(QString id, QString datetime) = 0;
	/**
	 * \returns return the status of the contact at timeStamp.
	 */
	virtual QString dcopStatus(QString id, int timeStamp) = 0;
	/**
	 * \returns the main status (most used status) of the contact id at date (not time) timeStamp. Will take the day where timeStamp is.
	 */
	virtual QString dcopMainStatus(QString id, int timeStamp) = 0;
};

#endif // STATISTICSDCOP_H
