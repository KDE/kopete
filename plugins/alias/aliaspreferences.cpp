/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kpushbutton.h>
#include <klistview.h>
#include <klocale.h>
#include <klineedit.h>
#include <kglobal.h>
#include <kgenericfactory.h>
#include <kautoconfig.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <qregexp.h>

#include "kopetecommandhandler.h"
#include "aliasdialogbase.h"
#include "aliasdialog.h"
#include "aliaspreferences.h"

typedef KGenericFactory<AliasPreferences> AliasPreferencesFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_alias, AliasPreferencesFactory( "kcm_kopete_alias" ) )

AliasPreferences::AliasPreferences( QWidget *parent, const char * /* name */, const QStringList &args )
: KCAutoConfigModule( AliasPreferencesFactory::instance(), parent, args )
{
	preferencesDialog = new AliasDialogBase( this );
	
	connect( preferencesDialog->addButton, SIGNAL(clicked()), this, SLOT( slotAddAlias() ) );
	connect( preferencesDialog->editButton, SIGNAL(clicked()), this, SLOT( slotEditAlias() ) );
	connect( preferencesDialog->deleteButton, SIGNAL(clicked()), this, SLOT( slotDeleteAliases() ) );

	connect( preferencesDialog->aliasList, SIGNAL(selectionChanged()), this, SLOT( slotCheckAliasSelected() ) );
	
	setMainWidget( preferencesDialog, "Alias Plugin" );

	load();
}

AliasPreferences::~AliasPreferences()
{

}

// reload configuration reading it from kopeterc
void AliasPreferences::load()
{
	KConfig *config = KGlobal::config();
	QMap<QString, QString> configMap = config->entryMap( QString::fromLatin1("AliasPlugin") );
	for( QMap<QString, QString>::Iterator it = configMap.begin(); it != configMap.end(); ++it )
	{
		if( !KopeteCommandHandler::commandHandler()->commandHandled( it.key() ) )
		{
			QString alias = it.key();
			QString command = it.data();
			addAlias( alias, command );
		}
	}

	KCAutoConfigModule::load();
	
	slotCheckAliasSelected();
}

// save list to kopeterc and creates map out of it
void AliasPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->deleteGroup( QString::fromLatin1("AliasPlugin") );
	config->setGroup( QString::fromLatin1("AliasPlugin") );
	
	QListViewItem *myChild = preferencesDialog->aliasList->firstChild();
        while( myChild )
	{
		config->writeEntry( myChild->text(0), myChild->text(1) );
		myChild = myChild->nextSibling();
        }

	KCAutoConfigModule::save();
}

void AliasPreferences::addAlias( QString &alias, QString &command )
{
	QRegExp spaces( QString::fromLatin1("\\s+") );
	
	if( alias.startsWith( QString::fromLatin1("/") ) )
		alias = alias.section( '/', 1 );
	if( command.startsWith( QString::fromLatin1("/") ) )
		command = command.section( '/', 1 );
	
	QString newAlias = command.section( spaces, 0, 0 );
	if( KopeteCommandHandler::commandHandler()->commandHandled( newAlias ) )
	{
		new QListViewItem( preferencesDialog->aliasList, alias, command );
	
		KopeteCommandHandler::commandHandler()->registerAlias(
			KopeteCommandHandler::commandHandler(), alias, command, QString::fromLatin1("Custom alias for %1").arg(command), KopeteCommandHandler::UserAlias );
	}
	else
	{
		KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. The command <b>%2</b> is not a valid Kopete command.</qt>").arg(alias).arg(newAlias),i18n("Could not add alias") );
	}
}

void AliasPreferences::slotAddAlias()
{
	AliasDialog addDialog;
	if( addDialog.exec() == QDialog::Accepted )
	{
		QString alias = addDialog.alias->text();
		QString command = addDialog.command->text();
		
		if( !KopeteCommandHandler::commandHandler()->commandHandled( alias ) )
			addAlias( alias, command );
		else
		{
			KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. This command is already being handled by either another alias or Kopete itself.</qt>").arg(alias),i18n("Could not add alias") );
		}
	}
}

void AliasPreferences::slotEditAlias()
{
	AliasDialog editDialog;
	QListViewItem *it = preferencesDialog->aliasList->selectedItems().first();
	if( it )
	{
		QString oldAlias = it->text(0);
		editDialog.alias->setText( oldAlias );
		editDialog.command->setText( it->text(1) );
		
		if( editDialog.exec() == QDialog::Accepted )
		{
			QString alias = editDialog.alias->text();
			if( alias.startsWith( QString::fromLatin1("/") ) )
				alias = alias.section( '/', 1 );
			QString command = editDialog.command->text();
			
			if( alias == oldAlias || !KopeteCommandHandler::commandHandler()->commandHandled( alias ) )
			{
				delete it;
				KopeteCommandHandler::commandHandler()->unregisterAlias( 
					KopeteCommandHandler::commandHandler(), oldAlias );
				addAlias( alias, command );
			}
			else
			{
				KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. This command is already being handled by either another alias or Kopete itself.</qt>").arg(alias),i18n("Could not add alias") );
			}
		}
	}
}

void AliasPreferences::slotDeleteAliases()
{
	if( KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete the selected aliases?"), i18n("Delete Aliases") ) == KMessageBox::Yes )
	{
		QPtrList< QListViewItem > items = preferencesDialog->aliasList->selectedItems();
		for( QListViewItem *i = items.first(); i; i = items.next() )
		{
			KopeteCommandHandler::commandHandler()->unregisterAlias( 
					KopeteCommandHandler::commandHandler(), i->text(0) );
			delete i;
		}
		
		save();
	}
}

void AliasPreferences::slotCheckAliasSelected()
{
	int numItems = preferencesDialog->aliasList->selectedItems().count();
	preferencesDialog->deleteButton->setEnabled( numItems > 0 );
	preferencesDialog->editButton->setEnabled( numItems  == 1 );
}

#include "aliaspreferences.moc"

