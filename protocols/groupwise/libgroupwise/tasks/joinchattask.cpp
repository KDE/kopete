/*
    Kopete Groupwise Protocol
    joinchattask.cpp - Join a Chat on the server, after having been invited.

    Copyright (c) 2005      SUSE Linux Products GmbH	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "joinchattask.h"

#include "gwerror.h"
#include "client.h"
#include "response.h"
#include "userdetailsmanager.h"

JoinChatTask::JoinChatTask(Task* parent): RequestTask(parent)
{
}

JoinChatTask::~JoinChatTask()
{
}

void JoinChatTask::join( const QString & displayName )
{
	m_displayName = displayName;
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, displayName ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "joinchat", lst );
}

bool JoinChatTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		client()->debug( "JoinChatTask::take()" );
		Response * response = dynamic_cast<Response *>( transfer );
		Field::FieldList responseFields = response->fields();
		// if the request was successful
		if ( response->resultCode() == GroupWise::None )
		{
			// extract the list of participants and store them
			Field::MultiField * participants = responseFields.findMultiField( Field::NM_A_FA_CONTACT_LIST );
			if ( participants )
			{
				Field::SingleField * contact = 0;
				Field::FieldList contactList = participants->fields();
				const Field::FieldListIterator end = contactList.end();
				for ( Field::FieldListIterator it = contactList.find( Field::NM_A_SZ_DN );
								  it != end;
								  it = contactList.find( ++it, Field::NM_A_SZ_DN ) )
				{
					contact = static_cast<Field::SingleField *>( *it );
					if ( contact )
					{
						// HACK: lowercased DN 
						QString dn = contact->value().toString().toLower();
						m_participants.append( dn );
						// need to ask for details for these contacts
					}
				}
			}
			else 
				setError( GroupWise::Protocol );
			
			// now, extract the list of pending invites and store them
			Field::MultiField * invitees = responseFields.findMultiField( Field::NM_A_FA_RESULTS );
			if ( invitees )
			{
				Field::SingleField * contact = 0;
				Field::FieldList contactList = invitees->fields();
				const Field::FieldListIterator end = contactList.end();
				for ( Field::FieldListIterator it = contactList.find( Field::NM_A_SZ_DN );
								  it != end;
								  it = contactList.find( ++it, Field::NM_A_SZ_DN ) )
				{
					contact = static_cast<Field::SingleField *>( *it );
					if ( contact )
					{
						// HACK: lowercased DN 
						QString dn = contact->value().toString().toLower();
						m_invitees.append( dn );
						// need to ask for details for these contacts
						//if ( !client()->userDetailsManager()->known( dn )  )
						//	; // don't request details for chatrooms, there could be too many
					}
				}
			}
			else 
				setError( GroupWise::Protocol );

			client()->debug( "JoinChatTask::finished()" );
			finished();	
		}
		else
			setError( response->resultCode() );
		return true;
	}
	else
		return false;
}

QStringList JoinChatTask::participants() const
{
	return m_participants;
}

QStringList JoinChatTask::invitees() const
{
	return m_invitees;
}

QString JoinChatTask::displayName() const
{
	return m_displayName;
}

#include "joinchattask.moc"
