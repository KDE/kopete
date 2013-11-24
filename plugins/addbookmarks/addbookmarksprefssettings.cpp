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
#include "addbookmarksprefssettings.h"

#include <kdebug.h>
#include <ksharedconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>

BookmarksPrefsSettings::BookmarksPrefsSettings(QObject *parent)
 : QObject(parent)
{
}


BookmarksPrefsSettings::~BookmarksPrefsSettings()
{
}

void BookmarksPrefsSettings::load()
{
	KSharedConfig::Ptr configfile = KGlobal::config();
	m_isfolderforeachcontact = Always;
	m_contactslist.clear();
	if( configfile->accessMode() == KConfigBase::NoAccess ){
		kDebug( 14501 ) << "load: failed to open config file for reading";
		return;
	}
	if( !configfile->hasGroup("Bookmarks Plugin") ){
		kDebug( 14501 ) << "load: no config found in file";
		return;
	}
	KConfigGroup group = configfile->group("Bookmarks Plugin");
	m_isfolderforeachcontact = (UseSubfolders)group.readEntry( "UseSubfolderForEachContact", 0 );
	m_contactslist = group.readEntry( "ContactsList", QStringList() );
}

void BookmarksPrefsSettings::save()
{
	KSharedConfig::Ptr configfile = KGlobal::config();

	if( configfile->accessMode() != KConfigBase::ReadWrite ){
		kDebug( 14501 ) << "save: failed to open config file for writing";
		return;
	}
	KConfigGroup group = configfile->group("Bookmarks Plugin");
	group.writeEntry( "UseSubfolderForEachContact", (int)m_isfolderforeachcontact );
	group.writeEntry( "ContactsList", m_contactslist );
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
			return ( m_contactslist.indexOf( nickname ) != -1 );
		case UnselectedContacts:
			return ( m_contactslist.indexOf( nickname ) == -1 );
		}
	}
	return false;
}

#include "addbookmarksprefssettings.moc"
