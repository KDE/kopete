/*
    Kopete Groupwise Protocol
    joinconferencetask.cpp - Join a conference on the server, after having been invited.

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

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

#include "joinconferencetask.h"

#include "gwerror.h"
#include "client.h"
#include "response.h"
#include "userdetailsmanager.h"

JoinConferenceTask::JoinConferenceTask(Task* parent): RequestTask(parent)
{
}

JoinConferenceTask::~JoinConferenceTask()
{
}

void JoinConferenceTask::join( const GroupWise::ConferenceGuid & guid )
{
	m_guid = guid;
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( Field::NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( Field::NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "joinconf", lst );
}

bool JoinConferenceTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		client()->debug( "JoinConferenceTask::take()" );
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
						if ( !client()->userDetailsManager()->known( dn )  )
							m_unknowns.append( dn );
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
						if ( !client()->userDetailsManager()->known( dn )  )
							m_unknowns.append( dn );
					}
				}
			}
			else 
				setError( GroupWise::Protocol );

			if ( m_unknowns.empty() )	// ready to chat
			{
				client()->debug( "JoinConferenceTask::finished()" );
				finished();
			}
			else								// need to get some more details first
			{
				client()->debug( "JoinConferenceTask::slotReceiveUserDetails(), requesting details" );
				connect( client()->userDetailsManager(), 
						SIGNAL(gotContactDetails(GroupWise::ContactDetails)),
						SLOT(slotReceiveUserDetails(GroupWise::ContactDetails)) );
				client()->userDetailsManager()->requestDetails( m_unknowns );
			}
		}
		else
			setError( response->resultCode() );
		return true;
	}
	else
		return false;
}

void JoinConferenceTask::slotReceiveUserDetails( const ContactDetails & details )
{
	client()->debug( QString( "JoinConferenceTask::slotReceiveUserDetails() - got %1" ).arg( details.dn ) );
	QStringList::Iterator it = m_unknowns.begin();
	QStringList::Iterator end = m_unknowns.end();
	for( ; it != end; ++it )
	{
		QString current = *it;
		client()->debug( QString( " - can we remove %1?" ).arg(current ) );
		if ( current == details.dn )
		{
			client()->debug( " - it is gone!" );
			m_unknowns.erase( it );
			break;
		}
	}
	client()->debug( QString( " - now %1 unknowns").arg( m_unknowns.count() ) );
	if ( m_unknowns.empty() )
	{
		client()->debug( " - finished()" );
		finished();
	}
// would be better to count the number of received details and listen to the getdetails task's error signal.
//	else
//	{
//		client()->debug( " - ERROR - we requested details for the list of chat participants/invitees, but the server did not send us all the details! - setting finished() anyway, so the chat can take place." );
//		finished();
//	}
}

QStringList JoinConferenceTask::participants() const
{
	return m_participants;
}

QStringList JoinConferenceTask::invitees() const
{
	return m_invitees;
}

GroupWise::ConferenceGuid JoinConferenceTask::guid() const
{
	return m_guid;
}

#include "joinconferencetask.moc"
