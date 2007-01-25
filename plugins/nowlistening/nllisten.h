/*
    nllisten.h

    Kopete Now Listening To plugin


    Copyright (c) 2007 by Tomer Haimovich <tomer.ha@gmail.com>

    Kopete    (c) 2002,2003,2004 by the Kopete developers  <kopete-devel@kde.org>
    
    Purpose: 
    This class abstracts the interface to Listen by
    implementing NLMediaPlayer

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef NLLISTEN_H
#define NLLISTEN_H

// We acknowledge the the dbus API is unstable
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/connection.h>

class NLlisten : public NLMediaPlayer
{
    public:
        NLlisten();
        virtual void update();
        
    private:
    	QString newTrack;
};

#endif

