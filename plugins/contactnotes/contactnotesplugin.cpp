/***************************************************************************
                          contactnotes.cpp  -  description
                             -------------------
    begin                : lun sep 16 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
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

#include "contactnotesedit.h"

#include "contactnotesplugin.h"

K_EXPORT_COMPONENT_FACTORY( kopete_contactnotes, KGenericFactory<ContactNotesPlugin> );

ContactNotesPlugin::ContactNotesPlugin( QObject *parent, const char *name,
	const QStringList &/*args*/ )
: KopetePlugin( parent, name )
{
	m_actionCollection=0L;
	m_actionEdit=0L;

	if ( pluginStatic_ )
		kdDebug(14302)<<"ContactNotesPlugin::ContactNotesPlugin : plugin already initialized"<<endl;
	else
		pluginStatic_ = this;

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

KActionCollection *ContactNotesPlugin::customContextMenuActions(KopeteMetaContact *m)
{
	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);
	m_actionEdit=new KAction( i18n("&Notes..."), "account", 0, this, SLOT (slotEditInfo()), m_actionCollection);

	m_actionCollection->insert(m_actionEdit);
	m_currentMetaContact=m;
	return m_actionCollection;
}

void ContactNotesPlugin::slotEditInfo()
{
	ContactNotesEdit *e=new ContactNotesEdit(m_currentMetaContact,this);
	connect(e,SIGNAL(notesChanged(QString,KopeteMetaContact*)),this,SLOT(setNotes(QString , KopeteMetaContact *)));
	e->show();
}


QString ContactNotesPlugin::notes(KopeteMetaContact *m)
{
	return m->pluginData( this, "notes" );
}

void ContactNotesPlugin::setNotes( const QString &n, KopeteMetaContact *m )
{
	m->setPluginData( this, "notes", n );
}

#include "contactnotesplugin.moc"

