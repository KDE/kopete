/*
    Kopete Yahoo Protocol
    Add a buddy to the Contactlist

    Copyright (c) 2005-2006 André Duffeck <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "modifybuddytask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"

#include <kdebug.h>

ModifyBuddyTask::ModifyBuddyTask(Task* parent) : Task(parent)
{
	kDebug(YAHOO_RAW_DEBUG) ;
}

ModifyBuddyTask::~ModifyBuddyTask()
{
}

bool ModifyBuddyTask::take( Transfer* transfer )
{
     if( !forMe( transfer ) )
	  return false;

     YMSGTransfer *t = static_cast<YMSGTransfer *>(transfer);

     bool success = t->firstParam(66) == "0";

     switch(t->service())
     {
     case Yahoo::ServiceBuddyAdd:
	  emit buddyAddResult(m_target, m_group, success);
	  break;
     case Yahoo::ServiceBuddyRemove:
	  emit buddyRemoveResult(m_target, m_group, success);
	  break;
     case Yahoo::ServiceBuddyChangeGroup:
	  emit buddyChangeGroupResult(m_target, m_group, success);
     default:
	  return false;
     }

     if(success)
	  setSuccess();
     else
	  setError();

     return true;
}

bool ModifyBuddyTask::forMe( const Transfer* transfer ) const
{
     const YMSGTransfer *t = 0L;
     t = dynamic_cast<const YMSGTransfer*>(transfer);
     
     if(!t)
	  return false;

     if( (t->service() == Yahoo::ServiceBuddyAdd ||
	  t->service() == Yahoo::ServiceBuddyRemove) &&
	 t->firstParam(7) == m_target)
     {
	  return true;
     }
     
     return false;
}

void ModifyBuddyTask::onGo()
{
	kDebug(YAHOO_RAW_DEBUG) ;

	switch( m_type )
	{
		case AddBuddy:
			addBuddy();
		break;
		case RemoveBuddy:
			removeBuddy();
		break;
		case MoveBuddy:
			moveBuddy();
		break;
	}
}

void ModifyBuddyTask::addBuddy()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceBuddyAdd);
	t->setId( client()->sessionID() );
	t->setParam( 65, m_group.toLocal8Bit() );
	t->setParam( 97, 1 );           // UTF-8
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 302,  319 );
	t->setParam( 300,  319 );
	t->setParam( 7,  m_target.toLocal8Bit() );
	t->setParam( 334,  0 );
	t->setParam( 301, 319 );
	t->setParam( 303, 319 );
	send( t );
}

void ModifyBuddyTask::removeBuddy()
{
	YMSGTransfer *t = new YMSGTransfer(Yahoo::ServiceBuddyRemove);
	t->setId( client()->sessionID() );
	t->setParam( 1, client()->userId().toLocal8Bit() );
	t->setParam( 7, m_target.toLocal8Bit() );
	t->setParam( 65, m_group.toLocal8Bit() );	
	send( t );
}

void ModifyBuddyTask::moveBuddy()
{
	YMSGTransfer *mov = new YMSGTransfer( Yahoo::ServiceBuddyChangeGroup );
	mov->setId( client()->sessionID() );
	mov->setParam( 1, client()->userId().toLocal8Bit() );
	mov->setParam( 302, 240 );
	mov->setParam( 300, 240 );
	mov->setParam( 7, m_target.toLocal8Bit() );
	mov->setParam( 224, m_oldGroup.toLocal8Bit() );
	mov->setParam( 264, m_group.toLocal8Bit() );
	mov->setParam( 301, 240 );
	mov->setParam( 303, 240 );
	send( mov );
}

void ModifyBuddyTask::setTarget( const QString &target )
{
	m_target = target;
}

void ModifyBuddyTask::setMessage( const QString &text )
{
	m_message = text;
}

void ModifyBuddyTask::setGroup( const QString &group )
{
	m_group = group;
}

void ModifyBuddyTask::setOldGroup( const QString &old )
{
	m_oldGroup = old;
}

void ModifyBuddyTask::setType( Type type )
{
	m_type = type;
}
