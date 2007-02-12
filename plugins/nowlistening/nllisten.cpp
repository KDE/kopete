/*
    nllisten.cpp

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

#include <kdebug.h>
#include <qstring.h>

#include "nlmediaplayer.h"
#include "nllisten.h"

NLlisten::NLlisten() : NLMediaPlayer()
{
    m_type = Audio;
    m_name = "Listen";
    newTrack = "";
}

void NLlisten::update()
{
	// create dbus connection variables
    DBusError err;
    dbus_error_init(&err);
    DBusConnection* conn;
    
	// connect to the session bus
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) { 
		fprintf(stderr, "Connection Error (%s)\n", err.message); 
		dbus_error_free(&err); 
	}
	if (NULL == conn) { 
		return;
	}
    
    // create dbus message variables
    DBusMessage* msg;
    DBusMessageIter args;
    DBusPendingCall* pending;
    
    msg = dbus_message_new_method_call("org.listen.dbus",
        "/org/listen/dbus", "org.listen.dbus", "current_playing");

    if (NULL == msg)
    {
        kdDebug ( 14307 ) << "Message null\n" << endl;
        return;
    }
    
    // send the message and wait for a reply
    if (!dbus_connection_send_with_reply (conn, msg, &pending, -1))
    {
        kdDebug ( 14307 ) << "Out of memory\n" << endl;
        return;
    }
    
    if (NULL == pending)
    {
        kdDebug ( 14307 ) << "Pending call null\n" << endl;
        return;
    }
    

    
    dbus_pending_call_block(pending);
    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg)
    {
        kdDebug ( 14307 ) << "Null reply\n" << endl;
        return;
    }
    
    dbus_pending_call_unref(pending);
    
    // get the reply
    
    const char* reply;
    if (!dbus_message_iter_init(msg, &args))
    {
    	kdDebug ( 14307 ) << "Message has no arguments\n" << endl;
    }
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
    {
    	kdDebug ( 14307 ) << "Expected string argument\n" << endl;
    }
    else
    {
    	dbus_message_iter_get_basic(&args, &reply);
    }
    
    // process the received string. it's built this way: <m_track> - (<m_album> - <m_artist>)
    
    if (reply != "")
    {
    	m_playing = true;
	    QString temp(reply);
		int pos = temp.find(" - (");
		if (pos > -1)
		{
			newTrack = temp.left(pos);
			temp = temp.right(temp.length() - pos - 4);
		}
		else
		{
			newTrack = "";
		}
		
		pos = temp.find(" - ");
		if (pos > -1)
		{
			m_album = temp.left(pos);
			temp = temp.right(temp.length() - pos - 3);
			m_artist = temp.left(temp.length() - 1);
		}
		else
		{
			m_album = "";
			m_artist = "";
		}
	}
	else
	{
		m_playing = false;
		m_track = "";
		m_artist = "";
		m_album = "";
	}
    
    if (newTrack != m_track)
    {
    	m_newTrack = true;
    	m_track = newTrack;
    }
    else
    {
    	m_newTrack = false;
    }
    
    
    dbus_connection_flush(conn);
    dbus_message_unref(msg);
}
