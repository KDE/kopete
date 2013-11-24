/*
    Kopete Groupwise Protocol
    ChatPropertiesTask.cpp - Task to update chatroom participant counts

    Copyright (c) 2005      SUSE Linux Products GmbH	 http://www.suse.com

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

#include "chatpropertiestask.h"

#include <kdebug.h>

#include "gwfield.h"
#include "response.h"

using namespace GroupWise;

ChatPropertiesTask::ChatPropertiesTask(Task* parent): RequestTask(parent)
{
}


ChatPropertiesTask::~ChatPropertiesTask()
{
}

void ChatPropertiesTask::setChat( const QString &displayName )
{
	Field::FieldList lst;
	m_chat = displayName;
	lst.append( new Field::SingleField( Field::NM_A_DISPLAY_NAME, 0, NMFIELD_TYPE_UTF8, m_chat ) );
	createTransfer( "chatproperties", lst );
}

bool ChatPropertiesTask::take( Transfer * transfer )
{
	if ( !forMe( transfer ) )
		return false;
	Response * response = dynamic_cast<Response *>( transfer );
	if ( !response )
		return false;
	if ( response->resultCode() )
	{
		setError( response->resultCode() );
		return true;
	}
	
	Field::FieldList responseFields = response->fields();
	Field::MultiField * resultsArray = responseFields.findMultiField( Field::NM_A_FA_CHAT );
	if ( !resultsArray )
	{
		setError( Protocol );
		return true;
	}
	
	Field::FieldList lst = resultsArray->fields();
	const Field::FieldListIterator end = lst.end();
	for ( Field::FieldListIterator it = lst.begin();
			 it != end;
			 ++it )
	{
		Field::SingleField * sf = dynamic_cast<Field::SingleField *>( *it );
		if ( sf )
		{
			if ( sf->tag() == Field::NM_A_DISPLAY_NAME )
				continue;
			else if ( sf->tag() == Field::NM_A_CHAT_OWNER_DN )
				m_ownerDn = sf->value().toString();
			else if ( sf->tag() == Field::NM_A_CHAT_CREATOR_DN )
				m_creatorDn= sf->value().toString();
			else if ( sf->tag() == Field::NM_A_DESCRIPTION )
				m_description =  sf->value().toString();
			else if ( sf->tag() == Field::NM_A_DISCLAIMER )
				m_disclaimer = sf->value().toString();
			else if ( sf->tag() == Field::NM_A_QUERY )
				m_query = sf->value().toString();
			else if ( sf->tag() == Field::NM_A_ARCHIVE )
				m_archive = sf->value().toString();
			else if ( sf->tag() == Field::NM_A_SZ_TOPIC )
				m_topic = sf->value().toString();
			else if ( sf->tag() == Field::NM_A_CREATION_TIME )
				m_creationTime.setTime_t( sf->value().toInt() );
			else if ( sf->tag() == Field::NM_A_UD_CHAT_RIGHTS )
				m_rights = sf->value().toInt();
			
		}
		else
		{
			Field::MultiField * mf = dynamic_cast<Field::MultiField *>( *it );
			if ( mf )
			{
				if ( mf->tag() == Field::NM_A_FA_CHAT_ACL )
				{
					Field::FieldList acl = mf->fields();
					const Field::FieldListIterator aclEnd = acl.end();
					for ( Field::FieldListIterator aclIt = acl.begin();
										 aclIt != aclEnd;
										 ++aclIt )
					{
						Field::MultiField * aclEntryFields = dynamic_cast<Field::MultiField *>( *aclIt );
						if ( aclEntryFields )
						{
							ChatContact entry;
							Field::FieldList entryFields = aclEntryFields->fields();
							Field::SingleField * sf; 
							if ( ( sf = entryFields.findSingleField ( Field::NM_A_SZ_DN ) ) )
								entry.dn = sf->value().toString();
							if ( ( sf = entryFields.findSingleField ( Field::NM_A_SZ_ACCESS_FLAGS ) ) )
								entry.chatRights = sf->value().toInt();
							//kDebug () << "got acl entry: " << entry.dn << ", " << entry.chatRights;
							m_aclEntries.append( entry );
						}
						
					}
				}
			}
		}
	}
	//kDebug () << "Got chatroom properties: " << m_chat << " : " << m_ownerDn << ", " << m_description << ", " << m_disclaimer << ", " << m_query << ", " << m_archive << ", " << m_topic << ", " << m_creatorDn << ", " << m_creationTime.toString() << ", " << m_rights;
	finished();
	return true;
}

QList< ChatContact > ChatPropertiesTask::aclEntries()
{
	return m_aclEntries;
}

#include "chatpropertiestask.moc"
