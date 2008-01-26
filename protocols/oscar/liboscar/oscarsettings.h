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
#ifndef OSCARSETTINGS_H
#define OSCARSETTINGS_H

#include "kopete_export.h"

namespace Oscar
{

/**
* This class is used to keep track of various persistant settings that the backend will always
* need to get from the frontend. This is the interface and storage class that will handle the
* settings.
* @author Matt Rogers
*/
class KOPETE_EXPORT Settings
{
public:
	Settings();
	~Settings();

	/* Web awareness settings */
	void setWebAware( bool webAware );
	bool webAware() const;
	
	/* Authorization settings */
	void setRequireAuth( bool require );
	bool requireAuth() const;
	
	/* Hide IP Settings */
	void setHideIP( bool hide );
	bool hideIP() const;
	
private:
	
	bool m_webAware;
	bool m_requireAuth;
	bool m_hideIP;
};

}

#endif

//kate: indent-mode csands; tab-width 4;

