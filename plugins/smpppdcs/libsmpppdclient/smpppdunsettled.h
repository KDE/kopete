/*
	smpppdunsettled.h

	Copyright (c) 2006      by Heiko Schaefer        <heiko@rangun.de>

	Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This program is free software; you can redistribute it and/or modify  *
	* it under the terms of the GNU General Public License as published by  *
	* the Free Software Foundation; version 2 of the License.               *
	*                                                                       *
	*************************************************************************
*/

#ifndef SMPPPDSMPPPDUNSETTLED_H
#define SMPPPDSMPPPDUNSETTLED_H

#include "smpppdstate.h"

namespace SMPPPD {

/**
	@author Heiko Schaefer <heiko@rangun.de>
*/
class Unsettled : public State
{
	Unsettled(const Unsettled&);
	Unsettled& operator=(const Unsettled&);

	Unsettled();
public:
    virtual ~Unsettled();

	static Unsettled * instance();
	
	virtual bool connect(Client * client, const QString& server, uint port = 3185);

private:
	QString make_response(const QString& chex, const QString& password) const;
	
private:
	static Unsettled * m_instance;
};

}

#endif
