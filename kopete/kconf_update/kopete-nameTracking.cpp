/*
    kconf_update app for updating the contactlist format ( <= 0.9.0) for MetaContacts to
    track the name of a subcontact.

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qdir.h>
#include <qtextstream.h>
#include <qdom.h>

#include <kstandarddirs.h>

static QTextStream qcerr( stderr, IO_WriteOnly );

int main()
{
	KInstance* inst = new KInstance( "Update script" );
	QString filename = locateLocal( "data", QString::fromLatin1( "kopete/contactlist.xml" ) );
	
	// Load contact list & save backup.
	QFile contactListFile( filename );
	contactListFile.open( IO_ReadOnly );
	QDomDocument contactList;
	contactList.setContent( &contactListFile );
	contactListFile.close();
	QDir().rename( filename, filename + QString::fromLatin1( ".bak" ) );
	
	// parse the XML file
	QDomElement list = contactList.documentElement();
	QDomElement mcElement = list.firstChild().toElement();
	
	while( !mcElement.isNull() )
	{
		
		// update all the MetaContacts
		if( mcElement.tagName() == QString::fromLatin1("meta-contact") )
		{
			QDomElement displayName;
			QDomElement subcontact;
			
			QDomElement elem = mcElement.firstChild().toElement();
			while( !elem.isNull() )
			{
				if( elem.tagName() == QString::fromLatin1( "display-name" ) )
					displayName = elem;
				if( elem.tagName() == QString::fromLatin1( "plugin-data" ) )
				{
					// check if it's a contact by checking for "protocol" substring in the tag,
					// and the presence of a contactId child element.
					QString pluginId = elem.attribute( QString::fromLatin1( "plugin-id" ) );
					bool isProtocol = ( pluginId.contains( "protocol", false ) > 0 ); // case-insensitive search
					bool hasContactId = false;
					QDomNode field = elem.firstChild();
					while( !field.isNull() )
					{
						QDomElement fieldElem = field.toElement();
						
						if( !fieldElem.isNull() &&
							fieldElem.tagName() == QString::fromLatin1( "plugin-data-field" ) && 
							fieldElem.attribute( QString::fromLatin1( "key" ) ) == QString::fromLatin1( "contactId" ) )
						{
							hasContactId = true;
							break;
						}
						field = field.nextSibling();
					}
					
					if( isProtocol && hasContactId )
						subcontact = elem;
				}
				
				elem = elem.nextSibling().toElement();
			} // end while
			
			// check if we're even tracking the subcontact's name
			// if displayName.isNull(), it simply won't find the attribute; no harm done
			bool tracking = 
				( displayName.attribute( QString::fromLatin1( "trackChildNameChanges" ),
				QString::fromLatin1( "0" ) ) == QString::fromLatin1( "1" ) );
			if( !displayName.isNull() && !subcontact.isNull() && tracking )
			{
				// collect info
				QString nsCID;
				QString nsPID;
				QString nsAID;

				nsPID = subcontact.attribute( QString::fromLatin1( "plugin-id" ) );
				QDomNode field = subcontact.firstChild();
				while( !field.isNull() )
				{
					QDomElement fieldElem = field.toElement();
						
					if( !fieldElem.isNull() && fieldElem.tagName() == QString::fromLatin1( "plugin-data-field" ) )
					{
						 if( fieldElem.attribute( QString::fromLatin1( "key" ) ) == QString::fromLatin1( "contactId" ) )
							 nsCID = fieldElem.text();
						 if( fieldElem.attribute( QString::fromLatin1( "key" ) ) == QString::fromLatin1( "accountId" ) )
							 nsAID = fieldElem.text();
					}
					field = field.nextSibling();
				}
				
				// create the tracking info
				displayName.setAttribute( QString::fromLatin1( "nameSourceContactId" ), nsCID );
				displayName.setAttribute( QString::fromLatin1( "nameSourcePluginId" ), nsPID );
				displayName.setAttribute( QString::fromLatin1( "nameSourceAccountId" ), nsAID );
			}
		}	
		
		mcElement = mcElement.nextSibling().toElement();
	}

	// Save converted contactlist
	contactListFile.open( IO_WriteOnly );
	QTextStream stream( &contactListFile );
	stream.setEncoding( QTextStream::UnicodeUTF8 );
	stream << contactList.toString( 4 );
	contactListFile.flush();
	contactListFile.close();

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

