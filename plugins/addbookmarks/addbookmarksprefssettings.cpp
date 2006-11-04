//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Roie Kerstein <sf_kersteinroie@bezeqint.net>, (C) 2004
//
// License: GPL v2
//
//
#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>

#include "addbookmarksprefssettings.h"

BookmarksPrefsSettings::BookmarksPrefsSettings(QObject *parent, const char *name)
 : QObject(parent, name)
{
	load();
}


BookmarksPrefsSettings::~BookmarksPrefsSettings()
{
}

void BookmarksPrefsSettings::load()
{
	KConfig * configfile = KGlobal::config();
	m_isfolderforeachcontact = Always;
	m_contactslist.clear();
	m_addbookmarksfromunknowns = false;
	if( configfile->getConfigState() == KConfigBase::NoAccess ){
		kdDebug( 14501 ) << "load: failed to open config file for reading" << endl;
		return;
	}
	if( !configfile->hasGroup("Bookmarks Plugin") ){
		kdDebug( 14501 ) << "load: no config found in file" << endl;
		return;
	}
	configfile->setGroup("Bookmarks Plugin");
	m_isfolderforeachcontact = (UseSubfolders)configfile->readNumEntry( "UseSubfolderForEachContact", 0 );
	m_contactslist = configfile->readListEntry( "ContactsList" );
	m_addbookmarksfromunknowns = configfile->readBoolEntry( "AddBookmarksFromUnknownContacts" );
}

void BookmarksPrefsSettings::save()
{
	KConfig * configfile = KGlobal::config();

	if( configfile->getConfigState() != KConfigBase::ReadWrite ){
		kdDebug( 14501 ) << "save: failed to open config file for writing" << endl;
		return;
	}
	configfile->setGroup( "Bookmarks Plugin" );
	configfile->writeEntry( "UseSubfolderForEachContact", (int)m_isfolderforeachcontact );
	configfile->writeEntry( "ContactsList", m_contactslist );
	configfile->writeEntry( "AddBookmarksFromUnknownContacts", m_addbookmarksfromunknowns );
	configfile->sync();
}

bool BookmarksPrefsSettings::useSubfolderForContact( QString nickname )
{
	if ( !nickname.isEmpty() )
	{
		switch( m_isfolderforeachcontact ){
		case Never:
			return false;
		case Always:
			return true;
		case SelectedContacts:
			return ( m_contactslist.find( nickname ) != m_contactslist.end() );
		case UnselectedContacts:
			return ( m_contactslist.find( nickname ) == m_contactslist.end() );
		}
	}
	return false;
}

void BookmarksPrefsSettings::setAddBookmarksFromUnknownContacts( bool addUntrusted )
{
    m_addbookmarksfromunknowns = addUntrusted;
}

#include "addbookmarksprefssettings.moc"
