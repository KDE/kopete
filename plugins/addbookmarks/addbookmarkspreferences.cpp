//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Roie Kerstein <sf_kersteinroie@bezeqint.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "addbookmarkspreferences.h"
#include "ui_addbookmarksprefsui.h"
#include "addbookmarksplugin.h"
#include <kgenericfactory.h>
#include <kopetepluginmanager.h>
#include <kopetecontactlist.h>
#include <qlayout.h>
#include <qnamespace.h>
#include <qradiobutton.h>
#include <QButtonGroup>
#include <QStringListModel>
#include <kopetemetacontact.h>

K_PLUGIN_FACTORY( BookmarksPreferencesFactory, registerPlugin<BookmarksPreferences>(); )
K_EXPORT_PLUGIN( BookmarksPreferencesFactory( "kcm_kopete_addbookmarks" ) )


BookmarksPreferences::BookmarksPreferences(QWidget *parent, const QVariantList &args)
 : KCModule(BookmarksPreferencesFactory::componentData(), parent, args)
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget();
	p_dialog = new Ui::BookmarksPrefsUI();
	p_dialog->setupUi( w );
	l->addWidget( w );

	p_buttonGroup = new QButtonGroup( this );
	p_buttonGroup->addButton( p_dialog->yesButton, BookmarksPrefsSettings::Always );
	p_buttonGroup->addButton( p_dialog->noButton, BookmarksPrefsSettings::Never );
	p_buttonGroup->addButton( p_dialog->onlySelectedButton, BookmarksPrefsSettings::SelectedContacts );
	p_buttonGroup->addButton( p_dialog->onlyNotSelectedButton, BookmarksPrefsSettings::UnselectedContacts );

	p_contactsListModel = new QStringListModel();
	p_dialog->contactList->setModel( p_contactsListModel );

	connect( p_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotSetStatusChanged()));
	connect( p_dialog->contactList, SIGNAL(activated(QModelIndex)),
	         this, SLOT(slotSetStatusChanged()));
	if(Kopete::PluginManager::self()->plugin("kopete_addbookmarks"))
           connect( this, SIGNAL(PreferencesChanged()), Kopete::PluginManager::self()->plugin("kopete_addbookmarks") , SLOT(slotReloadSettings()));
}


BookmarksPreferences::~BookmarksPreferences()
{
	delete p_dialog;
	delete p_contactsListModel;
}

void BookmarksPreferences::save()
{
	m_settings.setFolderForEachContact( (BookmarksPrefsSettings::UseSubfolders)p_buttonGroup->checkedId() );
	if ( m_settings.isFolderForEachContact() == BookmarksPrefsSettings::SelectedContacts ||
		 m_settings.isFolderForEachContact() == BookmarksPrefsSettings::UnselectedContacts )
	{
		QStringList list;
		QModelIndexList indexList = p_dialog->contactList->selectionModel()->selectedIndexes();
		foreach( const QModelIndex &index, indexList )
			list += p_contactsListModel->data( index, Qt::DisplayRole ).toString();

		m_settings.setContactsList( list );
	}
	m_settings.save();
	emit PreferencesChanged();
	emit KCModule::changed(false);
}

void BookmarksPreferences::slotSetStatusChanged()
{
	if ( p_buttonGroup->checkedId() == BookmarksPrefsSettings::Always ||
	     p_buttonGroup->checkedId() == BookmarksPrefsSettings::Never )
		p_dialog->contactList->setEnabled(false);
	else
		p_dialog->contactList->setEnabled(true);

	emit KCModule::changed(true);
}

void BookmarksPreferences::load()
{
	m_settings.load();
	QAbstractButton *button = p_buttonGroup->button( m_settings.isFolderForEachContact() );
	if ( button )
		button->setChecked( true );

	QStringList contactsList;
	foreach(Kopete::MetaContact *contact, Kopete::ContactList::self()->metaContacts())
	{
		contactsList << contact->displayName();
	}

	contactsList.sort();
	p_contactsListModel->setStringList( contactsList );

	p_dialog->contactList->setEnabled( m_settings.isFolderForEachContact() == BookmarksPrefsSettings::SelectedContacts ||
	                                   m_settings.isFolderForEachContact() == BookmarksPrefsSettings::UnselectedContacts );

	QItemSelectionModel *selectionModel = p_dialog->contactList->selectionModel();
	selectionModel->clearSelection();

	const QStringList selectedContactsList = m_settings.getContactsList();
	foreach( const QString &contact, selectedContactsList )
	{
		const int row = contactsList.indexOf( contact );
		if ( row != -1 )
		{
			QModelIndex index = p_contactsListModel->index( row, 0, QModelIndex() );
			selectionModel->select( index, QItemSelectionModel::Select );
		}
	}
	emit KCModule::changed(false);
}

#include "addbookmarkspreferences.moc"
