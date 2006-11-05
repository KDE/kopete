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
#include <kmessagebox.h>
#include <kconfig.h>
#include <qregexp.h>
#include <qlayout.h>
#include <kplugininfo.h>
#include <kiconloader.h>
#include <qpainter.h>

#include "kopetecommandhandler.h"
#include "kopetepluginmanager.h"
#include "kopeteaccount.h"
#include "kopeteprotocol.h"

#include "aliasdialogbase.h"
#include "editaliasdialog.h"
#include "aliaspreferences.h"

typedef KGenericFactory<AliasPreferences> AliasPreferencesFactory;

class AliasItem : public QListViewItem
{
	public:
		AliasItem( QListView *parent,
			uint number,
			const QString &alias,
			const QString &command, const ProtocolList &p ) :
		QListViewItem( parent, alias, command )
		{
			protocolList = p;
			id = number;
		}

		ProtocolList protocolList;
		uint id;

	protected:
		void paintCell( QPainter *p, const QColorGroup &cg,
			int column, int width, int align )
		{
			if ( column == 2 )
			{
				int cellWidth = width - ( protocolList.count() * 16 ) - 4;
				if ( cellWidth < 0 )
					cellWidth = 0;

				QListViewItem::paintCell( p, cg, column, cellWidth, align );

				// Draw the rest of the background
				QListView *lv = listView();
				if ( !lv )
					return;

				int marg = lv->itemMargin();
				int r = marg;
				const BackgroundMode bgmode = lv->viewport()->backgroundMode();
				const QColorGroup::ColorRole crole =
					QPalette::backgroundRoleFromMode( bgmode );
				p->fillRect( cellWidth, 0, width - cellWidth, height(),
					cg.brush( crole ) );

				if ( isSelected() && ( column == 0 || listView()->allColumnsShowFocus() ) )
				{
					p->fillRect( QMAX( cellWidth, r - marg ), 0,
						width - cellWidth - r + marg, height(),
						cg.brush( QColorGroup::Highlight ) );
					if ( isEnabled() || !lv )
						p->setPen( cg.highlightedText() );
					else if ( !isEnabled() && lv )
						p->setPen( lv->palette().disabled().highlightedText() );
				}

				// And last, draw the online status icons
				int mc_x = 0;

				for ( ProtocolList::Iterator it = protocolList.begin();
					it != protocolList.end(); ++it )
				{
					QPixmap icon = SmallIcon( (*it)->pluginIcon() );
					p->drawPixmap( mc_x + 4, height() - 16,
						icon );
					mc_x += 16;
				}
			}
			else
			{
				// Use Qt's own drawing
				QListViewItem::paintCell( p, cg, column, width, align );
			}
		}
};

class ProtocolItem : public QListViewItem
{
	public:
		ProtocolItem( QListView *parent, KPluginInfo *p ) :
		QListViewItem( parent, p->name() )
		{
			this->setPixmap( 0, SmallIcon( p->icon() ) );
			id = p->pluginName();
		}

		QString id;
};

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_alias, AliasPreferencesFactory( "kcm_kopete_alias" ) )

AliasPreferences::AliasPreferences( QWidget *parent, const char *, const QStringList &args )
	: KCModule( AliasPreferencesFactory::instance(), parent, args )
{
	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new AliasDialogBase( this );

	connect( preferencesDialog->addButton, SIGNAL(clicked()), this, SLOT( slotAddAlias() ) );
	connect( preferencesDialog->editButton, SIGNAL(clicked()), this, SLOT( slotEditAlias() ) );
	connect( preferencesDialog->deleteButton, SIGNAL(clicked()), this, SLOT( slotDeleteAliases() ) );
	connect( Kopete::PluginManager::self(), SIGNAL( pluginLoaded( Kopete::Plugin * ) ),
		this, SLOT( slotPluginLoaded( Kopete::Plugin * ) ) );

	connect( preferencesDialog->aliasList, SIGNAL(selectionChanged()),
		this, SLOT( slotCheckAliasSelected() ) );

	load();
}

AliasPreferences::~AliasPreferences()
{
	QListViewItem *myChild = preferencesDialog->aliasList->firstChild();
        while( myChild )
	{
		ProtocolList protocols = static_cast<AliasItem*>( myChild )->protocolList;
		for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
		{
			Kopete::CommandHandler::commandHandler()->unregisterAlias(
				*it,
				myChild->text(0)
			);
		}

		myChild = myChild->nextSibling();
	}
}

// reload configuration reading it from kopeterc
void AliasPreferences::load()
{
	KConfig *config = KGlobal::config();
	if( config->hasGroup( "AliasPlugin" ) )
	{
		config->setGroup("AliasPlugin");
		QStringList aliases = config->readListEntry("AliasNames");
		for( QStringList::Iterator it = aliases.begin(); it != aliases.end(); ++it )
		{
			uint aliasNumber = config->readUnsignedNumEntry( (*it) + "_id" );
			QString aliasCommand = config->readEntry( (*it) + "_command" );
			QStringList protocols = config->readListEntry( (*it) + "_protocols" );

			ProtocolList protocolList;
			for( QStringList::Iterator it2 = protocols.begin(); it2 != protocols.end(); ++it2 )
			{
				Kopete::Plugin *p = Kopete::PluginManager::self()->plugin( *it2 );
				protocolList.append( (Kopete::Protocol*)p );
			}

			addAlias( *it, aliasCommand, protocolList, aliasNumber );
		}

	}
	
	slotCheckAliasSelected();
}

void AliasPreferences::slotPluginLoaded( Kopete::Plugin *plugin )
{
	Kopete::Protocol *protocol = static_cast<Kopete::Protocol*>( plugin );
	if( protocol )
	{
		KConfig *config = KGlobal::config();
		if( config->hasGroup( "AliasPlugin" ) )
		{
			config->setGroup("AliasPlugin");
			QStringList aliases = config->readListEntry("AliasNames");
			for( QStringList::Iterator it = aliases.begin(); it != aliases.end(); ++it )
			{
				uint aliasNumber = config->readUnsignedNumEntry( (*it) + "_id" );
				QString aliasCommand = config->readEntry( (*it) + "_command" );
				QStringList protocols = config->readListEntry( (*it) + "_protocols" );

				for( QStringList::iterator it2 = protocols.begin(); it2 != protocols.end(); ++it2 )
				{
					if( *it2 == protocol->pluginId() )
					{
						QPair<Kopete::Protocol*, QString> pr( protocol, *it );
						if( protocolMap.find( pr ) == protocolMap.end() )
						{
							Kopete::CommandHandler::commandHandler()->registerAlias(
								protocol,
								*it,
								aliasCommand,
								QString::fromLatin1("Custom alias for %1").arg(aliasCommand),
								Kopete::CommandHandler::UserAlias
							);

							protocolMap.insert( pr, true );

							AliasItem *item = aliasMap[ *it ];
							if( item )
							{
								item->protocolList.append( protocol );
								item->repaint();
							}
							else
							{
								ProtocolList pList;
								pList.append( protocol );
								aliasMap.insert( *it, new AliasItem( preferencesDialog->aliasList, aliasNumber, *it, aliasCommand, pList ) );
							}
						}
					}
				}
			}
		}
	}
}

// save list to kopeterc and creates map out of it
void AliasPreferences::save()
{
	KConfig *config = KGlobal::config();
	config->deleteGroup( QString::fromLatin1("AliasPlugin") );
	config->setGroup( QString::fromLatin1("AliasPlugin") );

	QStringList aliases;
	AliasItem *item = (AliasItem*)preferencesDialog->aliasList->firstChild();
        while( item )
	{
		QStringList protocols;
		for( ProtocolList::Iterator it = item->protocolList.begin();
			it != item->protocolList.end(); ++it )
		{
			protocols += (*it)->pluginId();
		}

		aliases += item->text(0);

		config->writeEntry( item->text(0) + "_id", item->id );
		config->writeEntry( item->text(0) + "_command", item->text(1) );
		config->writeEntry( item->text(0) + "_protocols", protocols );

		item = (AliasItem*)item->nextSibling();
        }

	config->writeEntry( "AliasNames", aliases );
	config->sync();
	emit KCModule::changed(false);
}

void AliasPreferences::addAlias( QString &alias, QString &command, const ProtocolList &p, uint id )
{
	QRegExp spaces( QString::fromLatin1("\\s+") );

	if( alias.startsWith( QString::fromLatin1("/") ) )
		alias = alias.section( '/', 1 );
	if( command.startsWith( QString::fromLatin1("/") ) )
		command = command.section( '/', 1 );

	if( id == 0 )
	{
		if( preferencesDialog->aliasList->lastItem() )
			id = static_cast<AliasItem*>( preferencesDialog->aliasList->lastItem() )->id + 1;
		else
			id = 1;
	}

	QString newAlias = command.section( spaces, 0, 0 );

	aliasMap.insert( alias, new AliasItem( preferencesDialog->aliasList, id, alias, command, p ) );

	// count the number of arguments present in 'command'
	QRegExp rx( "(%\\d+)" );
	QStringList list;
	int pos = 0;
	while ( pos >= 0 ) {
		pos = rx.search( command, pos );
		if ( pos > -1 ) {
			list += rx.cap( 1 );
			pos  += rx.matchedLength();
		}
	}
	int argc = list.count();

	for( ProtocolList::ConstIterator it = p.begin(); it != p.end(); ++it )
	{
		Kopete::CommandHandler::commandHandler()->registerAlias(
			*it,
			alias,
			command,
			QString::fromLatin1("Custom alias for %1").arg(command),
			Kopete::CommandHandler::UserAlias,
			0,
			argc
		);

		protocolMap.insert( QPair<Kopete::Protocol*,QString>( *it, alias ), true );
	}
}

void AliasPreferences::slotAddAlias()
{
	EditAliasDialog addDialog;
	loadProtocols( &addDialog );
	addDialog.addButton->setText( i18n("&Add") );

	if( addDialog.exec() == QDialog::Accepted )
	{
		QString alias = addDialog.alias->text();
		if( alias.startsWith( QString::fromLatin1("/") ) )
			alias = alias.section( '/', 1 );

		if( alias.contains( QRegExp("[_=]") ) )
		{
			KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. An"
					" alias name cannot contain the characters \"_\" or \"=\"."
					"</qt>").arg(alias),i18n("Invalid Alias Name") );
		}
		else
		{
			QString command = addDialog.command->text();
			ProtocolList protocols = selectedProtocols( &addDialog );

			// Loop through selected protocols

			for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
			{

				// And check if they already have the command enabled

				if( Kopete::CommandHandler::commandHandler()->commandHandledByProtocol( alias, *it ) )
				{
					KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. This "
						"command is already being handled by either another alias or "
						"Kopete itself.</qt>").arg(alias), i18n("Could Not Add Alias") );
					return;
				}
			}
			addAlias( alias, command, protocols );
			emit KCModule::changed(true); 

		}
	}
}

const ProtocolList AliasPreferences::selectedProtocols( EditAliasDialog *dialog )
{
	ProtocolList protocolList;
	QListViewItem *item = dialog->protocolList->firstChild();

	while( item )
	{
		if( item->isSelected() )
		{

			// If you dont have the selected protocol enabled, Kopete::PluginManager::self()->plugin
			// will return NULL, check for that

			if(Kopete::PluginManager::self()->plugin( static_cast<ProtocolItem*>(item)->id) )
				protocolList.append( (Kopete::Protocol*)
					Kopete::PluginManager::self()->plugin( static_cast<ProtocolItem*>(item)->id )
			);
		}
		item = item->nextSibling();
	}

	return protocolList;
}

void AliasPreferences::loadProtocols( EditAliasDialog *dialog )
{
	QValueList<KPluginInfo*> plugins = Kopete::PluginManager::self()->availablePlugins("Protocols");
	for( QValueList<KPluginInfo*>::Iterator it = plugins.begin(); it != plugins.end(); ++it )
	{
		ProtocolItem *item = new ProtocolItem( dialog->protocolList, *it );
		itemMap[ (Kopete::Protocol*)Kopete::PluginManager::self()->plugin( (*it)->pluginName() ) ] = item;
	}
}

void AliasPreferences::slotEditAlias()
{
	EditAliasDialog editDialog;
	loadProtocols( &editDialog );

	QListViewItem *item = preferencesDialog->aliasList->selectedItems().first();
	if( item )
	{
		QString oldAlias = item->text(0);
		editDialog.alias->setText( oldAlias );
		editDialog.command->setText( item->text(1) );
		ProtocolList protocols = static_cast<AliasItem*>( item )->protocolList;
		for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
		{
			itemMap[ *it ]->setSelected( true );
		}

		if( editDialog.exec() == QDialog::Accepted )
		{
			QString alias = editDialog.alias->text();
			if( alias.startsWith( QString::fromLatin1("/") ) )
				alias = alias.section( '/', 1 );
			if( alias.contains( QRegExp("[_=]") ) )
			{
				KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. An"
						" alias name cannot contain the characters \"_\" or \"=\"."
						"</qt>").arg(alias),i18n("Invalid Alias Name") );
			}
			else
			{
				QString command = editDialog.command->text();

				if( alias == oldAlias )
				{
                                        for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
                                        {
                                                Kopete::CommandHandler::commandHandler()->unregisterAlias(
                                                        *it,
                                                        oldAlias
                                                );
                                        }


                                        ProtocolList selProtocols = selectedProtocols( &editDialog );

					for( ProtocolList::Iterator it = selProtocols.begin(); it != selProtocols.end(); ++it )
					{
						if( Kopete::CommandHandler::commandHandler()->commandHandledByProtocol( alias, *it ) )
						{
							KMessageBox::error( this, i18n("<qt>Could not add alias <b>%1</b>. This "
							"command is already being handled by either another alias or "
							"Kopete itself.</qt>").arg(alias), i18n("Could Not Add Alias") );
						return;
						}
					}

					delete item;

					addAlias( alias, command, selProtocols );
					emit KCModule::changed(true);
				}
			}
		}
	}
}

void AliasPreferences::slotDeleteAliases()
{
	if( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to delete the selected aliases?"), i18n("Delete Aliases"), KGuiItem(i18n("Delete"), "editdelete") ) == KMessageBox::Continue )
	{
		QPtrList< QListViewItem > items = preferencesDialog->aliasList->selectedItems();
		for( QListViewItem *i = items.first(); i; i = items.next() )
		{
			ProtocolList protocols = static_cast<AliasItem*>( i )->protocolList;
			for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				Kopete::CommandHandler::commandHandler()->unregisterAlias(
					*it,
					i->text(0)
				);

				protocolMap.erase( QPair<Kopete::Protocol*,QString>( *it, i->text(0) ) );
			}

			aliasMap.erase( i->text(0) );
			delete i;
			emit KCModule::changed(true);
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

