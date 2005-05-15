/***************************************************************************
                          contactnotes.cpp  -  description
                             -------------------
    begin                : lun sep 16 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include "kopetemetacontact.h"
#include "kopetecontactlist.h"

#include "contactnotesedit.h"

#include "contactnotesplugin.h"

typedef KGenericFactory<ContactNotesPlugin> ContactNotesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_contactnotes, ContactNotesPluginFactory( "kopete_contactnotes" )  )

ContactNotesPlugin::ContactNotesPlugin( QObject *parent, const char *name, const QStringList & /* args */ )
: Kopete::Plugin( ContactNotesPluginFactory::instance(), parent, name )
{
	if ( pluginStatic_ )
		kdDebug(14302)<<"ContactNotesPlugin::ContactNotesPlugin : plugin already initialized"<<endl;
	else
		pluginStatic_ = this;

	KAction *m_actionEdit=new KAction( i18n("&Notes"), "identity", 0, this, SLOT (slotEditInfo()), actionCollection() , "editContactNotes");
	connect ( Kopete::ContactList::self() , SIGNAL( metaContactSelected(bool)) , m_actionEdit , SLOT(setEnabled(bool)));
	m_actionEdit->setEnabled(Kopete::ContactList::self()->selectedMetaContacts().count()==1 );

	setXMLFile("contactnotesui.rc");
}

ContactNotesPlugin::~ContactNotesPlugin()
{
	pluginStatic_ = 0L;
}

ContactNotesPlugin* ContactNotesPlugin::plugin()
{
	return pluginStatic_ ;
}

ContactNotesPlugin* ContactNotesPlugin::pluginStatic_ = 0L;


void ContactNotesPlugin::slotEditInfo()
{
	Kopete::MetaContact *m=Kopete::ContactList::self()->selectedMetaContacts().first();
	if(!m)
		return;
	ContactNotesEdit *e=new ContactNotesEdit(m,this);
	connect( e, SIGNAL( notesChanged( const QString, Kopete::MetaContact*) ),this,
			SLOT( setNotes( const QString, Kopete::MetaContact * ) ) );
	e->show();
}


QString ContactNotesPlugin::notes(Kopete::MetaContact *m)
{
	return m->pluginData( this, "notes" );
}

void ContactNotesPlugin::setNotes( const QString n, Kopete::MetaContact *m )
{
	m->setPluginData( this, "notes", n );
}

#include "contactnotesplugin.moc"

