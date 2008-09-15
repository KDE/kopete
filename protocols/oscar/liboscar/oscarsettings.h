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

#include "liboscar_export.h"

namespace Oscar
{

/**
* This class is used to keep track of various persistant settings that the backend will always
* need to get from the frontend. This is the interface and storage class that will handle the
* settings.
* @author Matt Rogers
*/
class LIBOSCAR_EXPORT Settings
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
	
	/* proxy Settings */
	void setFileProxy( bool proxy );
	bool fileProxy() const;
	
	/* port range settings */
	void setFirstPort( int port );
	int firstPort() const;
	void setLastPort( int port );
	int lastPort() const;
	
	/* timeout in seconds */
	void setTimeout( int time );
	int timeout() const;
	
private:
	
	bool m_webAware;
	bool m_requireAuth;
	bool m_hideIP;
	bool m_fileProxy;
	int  m_firstPort;
	int  m_lastPort;
	int  m_timeout;
};

}

#endif

//kate: indent-mode csands; tab-width 4;

