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
#include <kconfig.h>

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
	KConfig configfile(QString::fromLatin1("kopeterc") , true);
	m_isfolderforeachcontact = Yes;
	m_contactslist.clear();
	if( configfile.getConfigState() == KConfigBase::NoAccess ){
		kdDebug( 14501 ) << "load: failed to open config file for reading" << endl;
		return;
	}
	if( !configfile.hasGroup("Bookmarks Plugin") ){
		kdDebug( 14501 ) << "load: no config found in file" << endl;
		return;
	}
	configfile.setGroup("Bookmarks Plugin");
	m_isfolderforeachcontact = (UseSubfolders)configfile.readNumEntry("UseSubfolderForEachContact", 0);
	m_contactslist = configfile.readListEntry("ContactsList");
}

void BookmarksPrefsSettings::save()
{
	KConfig configfile(QString::fromLatin1("kopeterc"));
	
	if( configfile.getConfigState() != KConfigBase::ReadWrite ){
		kdDebug( 14501 ) << "save: failed to open config file for writing" << endl;
		return;
	}
	configfile.setGroup("Bookmarks Plugin");
	configfile.writeEntry("UseSubfolderForEachContact", (int)m_isfolderforeachcontact);
	configfile.writeEntry("ContactsList", m_contactslist);
	configfile.sync();
}

bool BookmarksPrefsSettings::isUseSubfolderForContact( QString nickname )
{
	switch( m_isfolderforeachcontact ){
	case Yes:
		return true;
	case No:
		return false;
	case OnlyContactsInList:
		return (m_contactslist.find(nickname) != m_contactslist.end());
	case OnlyContactsNotInList:
		return (m_contactslist.find(nickname) == m_contactslist.end());
	}
	return false;
}

#include "addbookmarksprefssettings.moc"
