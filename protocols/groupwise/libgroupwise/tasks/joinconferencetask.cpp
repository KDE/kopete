//
// C++ Implementation: joinconferencetask
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "gwerror.h"
#include "client.h"
#include "response.h"
#include "userdetailsmanager.h"

#include "joinconferencetask.h"

JoinConferenceTask::JoinConferenceTask(Task* parent): RequestTask(parent)
{
}

JoinConferenceTask::~JoinConferenceTask()
{
}

void JoinConferenceTask::join( const QString & guid )
{
	m_guid = guid;
	Field::FieldList lst, tmp;
	tmp.append( new Field::SingleField( NM_A_SZ_OBJECT_ID, 0, NMFIELD_TYPE_UTF8, guid ) );
	lst.append( new Field::MultiField( NM_A_FA_CONVERSATION, NMFIELD_METHOD_VALID, 0, NMFIELD_TYPE_ARRAY, tmp ) );
	createTransfer( "joinconf", lst );
}

bool JoinConferenceTask::take( Transfer * transfer )
{
	if ( forMe( transfer ) )
	{
		qDebug( "JoinConferenceTask::take()" );
		Response * response = dynamic_cast<Response *>( transfer );
		Field::FieldList responseFields = response->fields();
		// if the request was successful
		if ( response->resultCode() == GroupWise::None )
		{
			// extract the list of participants and store them
			Field::MultiField * participants = responseFields.findMultiField( NM_A_FA_CONTACT_LIST );
			if ( participants )
			{
				Field::SingleField * contact = 0;
				Field::FieldList contactList = participants->fields();
				const Field::FieldListIterator end = contactList.end();
				for ( Field::FieldListIterator it = contactList.find( NM_A_SZ_DN );
					 it != end;
					 it = contactList.find( ++it, NM_A_SZ_DN ) )
				{
					contact = static_cast<Field::SingleField *>( *it );
					if ( contact )
					{
						// HACK: lowercased DN 
					 	QString dn = contact->value().toString().lower();
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
			Field::MultiField * invitees = responseFields.findMultiField( NM_A_FA_RESULTS );
			if ( invitees )
			{
				Field::SingleField * contact = 0;
				Field::FieldList contactList = invitees->fields();
				const Field::FieldListIterator end = contactList.end();
				for ( Field::FieldListIterator it = contactList.find( NM_A_SZ_DN );
					 it != end;
					 it = contactList.find( ++it, NM_A_SZ_DN ) )
				{
					contact = static_cast<Field::SingleField *>( *it );
					if ( contact )
					{
						// HACK: lowercased DN 
					 	QString dn = contact->value().toString().lower();
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
				qDebug( "JoinConferenceTask::finished()" );
				finished();	
			}
			else								// need to get some more details first
			{
				qDebug( "JoinConferenceTask::slotReceiveUserDetails(), requesting details" );
				connect( client()->userDetailsManager(), 
						SIGNAL( gotContactDetails( const GroupWise::ContactDetails & ) ),
						SLOT( slotReceiveUserDetails( const GroupWise::ContactDetails & ) ) );
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
	qDebug( "JoinConferenceTask::slotReceiveUserDetails() - got %s", details.dn.ascii() );
	QStringList::Iterator it = m_unknowns.begin();
	QStringList::Iterator end = m_unknowns.end();
	while( it != end )
	{
		QString current = *it;
		++it;
		qDebug( " - can we remove %s?", current.ascii() );
		if ( current == details.dn )
		{
			qDebug( " - it's gone!" );
			m_unknowns.remove( current );
			break;
		}
	}
	qDebug( " - now %u unknowns", m_unknowns.count() );
	if ( m_unknowns.empty() )
	{
		qDebug( " - finished()" );
		finished();
	}
	else
	{
		qDebug( " - ERROR - we requested details for the list of chat participants/invitees, but the server did not send us all the details! - setting finished() anyway, so the chat can take place." );
		finished();
	}
}

QStringList JoinConferenceTask::participants() const
{
	return m_participants;
}

QStringList JoinConferenceTask::invitees() const
{
	return m_invitees;
}

QString JoinConferenceTask::guid() const
{
	return m_guid;
}

#include "joinconferencetask.moc"
