/*
    Kopete Oscar Protocol
    Oscar Backend Setting Storage

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "oscarsettings.h"

namespace Oscar
{

Settings::Settings()
{
}


Settings::~Settings()
{
}

void Settings::setWebAware( bool aware )
{
	m_webAware = aware;
}

bool Settings::webAware() const
{
	return m_webAware;
}

void Settings::setRequireAuth( bool require )
{
	m_requireAuth = require;
}

bool Settings::requireAuth() const
{
	return m_requireAuth;
}

void Settings::setHideIP( bool hide )
{
	m_hideIP = hide;
}

bool Settings::hideIP() const
{
	return m_hideIP;
}


	


}

//kate: indent-mode csands; tab-width 4;

