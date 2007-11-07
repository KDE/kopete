/*
    kconf_update app for updating the contact list format ( <= 0.9.0) for MetaContacts to
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
#include <kcomponentdata.h>

static QTextStream qcerr( stderr, QIODevice::WriteOnly );

int main()
{
	KComponentData inst( "Update script" );
	QString filename = KStandardDirs::locateLocal( "data", QLatin1String( "kopete/contactlist.xml" ) );
	
	// Load contact list & save backup.
	QFile contactListFile( filename );
	contactListFile.open( QIODevice::ReadOnly );
	QDomDocument contactList;
	contactList.setContent( &contactListFile );
	contactListFile.close();
	QDir().rename( filename, filename + QLatin1String( ".bak" ) );
	
	// parse the XML file
	QDomElement list = contactList.documentElement();
	QDomElement mcElement = list.firstChild().toElement();
	
	while( !mcElement.isNull() )
	{
		
		// update all the MetaContacts
		if( mcElement.tagName() == QLatin1String("meta-contact") )
		{
			QDomElement displayName;
			QDomElement subcontact;
			
			QDomElement elem = mcElement.firstChild().toElement();
			while( !elem.isNull() )
			{
				if( elem.tagName() == QLatin1String( "display-name" ) )
					displayName = elem;
				if( elem.tagName() == QLatin1String( "plugin-data" ) )
				{
					// check if it's a contact by checking for "protocol" substring in the tag,
					// and the presence of a contactId child element.
					QString pluginId = elem.attribute( QLatin1String( "plugin-id" ) );
					bool isProtocol = ( pluginId.contains( "protocol", false ) > 0 ); // case-insensitive search
					bool hasContactId = false;
					QDomNode field = elem.firstChild();
					while( !field.isNull() )
					{
						QDomElement fieldElem = field.toElement();
						
						if( !fieldElem.isNull() &&
							fieldElem.tagName() == QLatin1String( "plugin-data-field" ) && 
							fieldElem.attribute( QLatin1String( "key" ) ) == QLatin1String( "contactId" ) )
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
				( displayName.attribute( QLatin1String( "trackChildNameChanges" ),
				QLatin1String( "0" ) ) == QLatin1String( "1" ) );
			if( !displayName.isNull() && !subcontact.isNull() && tracking )
			{
				// collect info
				QString nsCID;
				QString nsPID;
				QString nsAID;

				nsPID = subcontact.attribute( QLatin1String( "plugin-id" ) );
				QDomNode field = subcontact.firstChild();
				while( !field.isNull() )
				{
					QDomElement fieldElem = field.toElement();
						
					if( !fieldElem.isNull() && fieldElem.tagName() == QLatin1String( "plugin-data-field" ) )
					{
						 if( fieldElem.attribute( QLatin1String( "key" ) ) == QLatin1String( "contactId" ) )
							 nsCID = fieldElem.text();
						 if( fieldElem.attribute( QLatin1String( "key" ) ) == QLatin1String( "accountId" ) )
							 nsAID = fieldElem.text();
					}
					field = field.nextSibling();
				}
				
				// create the tracking info
				displayName.setAttribute( QLatin1String( "nameSourceContactId" ), nsCID );
				displayName.setAttribute( QLatin1String( "nameSourcePluginId" ), nsPID );
				displayName.setAttribute( QLatin1String( "nameSourceAccountId" ), nsAID );
			}
		}	
		
		mcElement = mcElement.nextSibling().toElement();
	}

	// Save converted contact list
	contactListFile.open( QIODevice::WriteOnly );
	QTextStream stream( &contactListFile );
	stream.setCodec(QTextCodec::codecForName("UTF-8"));
	stream << contactList.toString( 4 );
	contactListFile.flush();
	contactListFile.close();

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

