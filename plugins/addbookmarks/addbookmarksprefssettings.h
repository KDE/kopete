//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: Roie Kerstein <sf_kersteinroie@bezeqint.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADDBOOKMARKSPREFSSETTINGS_H
#define ADDBOOKMARKSPREFSSETTINGS_H

#include <qobject.h>
#include <qstringlist.h>

/**
@author Roie Kerstein <sf_kersteinroie@bezeqint.net>
*/
class BookmarksPrefsSettings : public QObject
{
Q_OBJECT
public:
    enum UseSubfolders { Yes=0, No=1, OnlyContactsInList=2, OnlyContactsNotInList=3 };

    BookmarksPrefsSettings(QObject *parent = 0, const char *name = 0);

    ~BookmarksPrefsSettings();
    
    void load();
    void save();
    UseSubfolders isFolderForEachContact() {return m_isfolderforeachcontact;}
    void setFolderForEachContact(UseSubfolders val) {m_isfolderforeachcontact = val;}
    bool isUseSubfolderForContact(QString nickname);
    QStringList getContactsList() {return m_contactslist;}
    void setContactsList(QStringList list) {m_contactslist = list;}
    
private:
    bool m_folderPerContact;
    UseSubfolders m_isfolderforeachcontact;
    QStringList m_contactslist;

};

#endif
