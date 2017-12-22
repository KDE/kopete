/*
   Kopete Oscar Protocol
   rateinfotask.h - Fetch the rate class information

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

#ifndef RATEINFOTASK_H
#define RATEINFOTASK_H

#include "task.h"
#include <QList>
#include "liboscar_export.h"

class RateClass;
using namespace Oscar;

/**
@author Matt Rogers
*/
class LIBOSCAR_EXPORT RateInfoTask : public Task
{
Q_OBJECT
public:
	RateInfoTask( Task* parent );
	~RateInfoTask();
	bool take( Transfer* transfer ) Q_DECL_OVERRIDE;
	static QList<RateClass*> parseRateClasses(Buffer *);

protected:

	bool forMe( const Transfer* transfer ) const Q_DECL_OVERRIDE;
	void onGo() Q_DECL_OVERRIDE;

Q_SIGNALS:
	void gotRateLimits();

private Q_SLOTS:

	//! Send the rate info request (SNAC 0x01, 0x06)
	void sendRateInfoRequest();

	//! Handle the rate info response (SNAC 0x01, 0x07)
	void handleRateInfoResponse();

	//! Acknowledge the rate information
	void sendRateInfoAck();

private:
	bool m_needRateAck;
	QList<int> m_rateGroups;
};

#endif
