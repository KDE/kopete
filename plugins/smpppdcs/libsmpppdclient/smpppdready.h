/*
	smpppdready.h

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

#ifndef SMPPPDREADY_H
#define SMPPPDREADY_H

#include "smpppdstate.h"

namespace SMPPPD {

/**
	@author Heiko Schaefer <heiko@rangun.de>
*/
class Ready : public State
{
	Ready(const Ready&);
	Ready& operator=(const Ready&);

	Ready();
	
public:
    virtual ~Ready();

	static Ready * instance();
	
	virtual void disconnect(Client * client);
	virtual QStringList getInterfaceConfigurations(Client * client);
	virtual bool statusInterface(Client * client, const QString& ifcfg);
	
private:
	static Ready * m_instance;
};

};

#endif
