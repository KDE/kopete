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

AddBookmarksPrefsSettings::AddBookmarksPrefsSettings(QObject *parent, const char *name)
 : QObject(parent, name)
{
	load();
}


AddBookmarksPrefsSettings::~AddBookmarksPrefsSettings()
{
}

void AddBookmarksPrefsSettings::load()
{
	KConfig configfile(QString::fromLatin1("kopeterc") , true);
	m_isfolderforeachcontact = Yes;
	m_contactslist.clear();
	if( configfile.getConfigState() == KConfigBase::NoAccess ){
		kdDebug( 14501 ) << "load: failed to open config file for reading" << endl;
		return;
	}
	if( !configfile.hasGroup("AddBookmarks Plugin") ){
		kdDebug( 14501 ) << "load: no config found in file" << endl;
		return;
	}
	configfile.setGroup("AddBookmarks Plugin");
	m_isfolderforeachcontact = (UseSubfolders)configfile.readNumEntry("UseSubfolderForEachContact", 0);
	m_contactslist = configfile.readListEntry("ContactsList");
}

void AddBookmarksPrefsSettings::save()
{
	KConfig configfile(QString::fromLatin1("kopeterc"));
	
	if( configfile.getConfigState() != KConfigBase::ReadWrite ){
		kdDebug( 14501 ) << "save: failed to open config file for writing" << endl;
		return;
	}
	configfile.setGroup("AddBookmarks Plugin");
	configfile.writeEntry("UseSubfolderForEachContact", (int)m_isfolderforeachcontact);
	configfile.writeEntry("ContactsList", m_contactslist);
	configfile.sync();
}

bool AddBookmarksPrefsSettings::isUseSubfolderForContact( QString nickname )
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
}

#include "addbookmarksprefssettings.moc"
