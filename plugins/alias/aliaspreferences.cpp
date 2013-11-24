/*
 aliaspreferences.cpp  -  Alias Plugin Preferences

 Copyright (c) 2003-2004      by the Kopete developers  <kopete-devel@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/

#define QT3_SUPPORT

#include "aliaspreferences.h"

#include <kpushbutton.h>
#include <k3listview.h>
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

#include "ui_aliasdialogbase.h"
#include "editaliasdialog.h"

K_PLUGIN_FACTORY( AliasPreferencesFactory, registerPlugin<AliasPreferences>(); )
K_EXPORT_PLUGIN( AliasPreferencesFactory( "kcm_kopete_alias" ) )

class AliasItem : public Q3ListViewItem
{
	public:
		AliasItem( Q3ListView *parent,
			uint number,
			const QString &alias,
			const QString &command, const ProtocolList &p ) :
		Q3ListViewItem( parent, alias, command )
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

				Q3ListViewItem::paintCell( p, cg, column, cellWidth, align );

				// Draw the rest of the background
				Q3ListView *lv = listView();
				if ( !lv )
					return;

				int marg = lv->itemMargin();
				int r = marg;
				// QPalette(backgroundRole(())
				QPalette bgmode = QPalette( lv->viewport()->backgroundRole() );
				p->fillRect( cellWidth, 0, width - cellWidth, height(),
					cg.brush( bgmode.currentColorGroup(), QPalette::Background ) );

				if ( isSelected() && ( column == 0 || listView()->allColumnsShowFocus() ) )
				{
					p->fillRect( qMax( cellWidth, r - marg ), 0,
						width - cellWidth - r + marg, height(),
						cg.brush( QPalette::Highlight ) );
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
				Q3ListViewItem::paintCell( p, cg, column, width, align );
			}
		}
};

class ProtocolItem : public Q3ListViewItem
{
	public:
		ProtocolItem( Q3ListView *parent, const KPluginInfo &p ) :
		Q3ListViewItem( parent, p.name() )
		{
			this->setPixmap( 0, SmallIcon( p.icon() ) );
			id = p.pluginName();
		}

		QString id;
};

AliasPreferences::AliasPreferences( QWidget *parent, const QVariantList &args )
	: KCModule( AliasPreferencesFactory::componentData(), parent, args )
{
	QVBoxLayout* l = new QVBoxLayout( this );
	QWidget* w = new QWidget;
	preferencesDialog = new Ui::AliasDialogBase;
	preferencesDialog->setupUi( w );
	l->addWidget( w );

	connect( preferencesDialog->addButton, SIGNAL(clicked()), this, SLOT(slotAddAlias()) );
	connect( preferencesDialog->editButton, SIGNAL(clicked()), this, SLOT(slotEditAlias()) );
	connect( preferencesDialog->deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteAliases()) );
	connect( Kopete::PluginManager::self(), SIGNAL(pluginLoaded(Kopete::Plugin*)),
		this, SLOT(slotPluginLoaded(Kopete::Plugin*)) );

	connect( preferencesDialog->aliasList, SIGNAL(selectionChanged()),
		this, SLOT(slotCheckAliasSelected()) );
}

AliasPreferences::~AliasPreferences()
{
	Q3ListViewItem *myChild = preferencesDialog->aliasList->firstChild();
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
	delete preferencesDialog;
}

// reload configuration reading it from kopeterc
void AliasPreferences::load()
{
	KConfigGroup config(KGlobal::config(), "AliasPlugin");
	if( config.exists() )
	{
		QStringList aliases = config.readEntry("AliasNames", QStringList() );
		for( QStringList::Iterator it = aliases.begin(); it != aliases.end(); ++it )
		{
			int aliasNumber = config.readEntry( (*it) + "_id", 0 );
			QString aliasCommand = config.readEntry( (*it) + "_command", QString() );
			QStringList protocols = config.readEntry( (*it) + "_protocols", QStringList() );

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
		KConfigGroup config(KGlobal::config(), "AliasPlugin");
		if( config.exists() )
		{
			QStringList aliases = config.readEntry("AliasNames", QStringList());
			for( QStringList::Iterator it = aliases.begin(); it != aliases.end(); ++it )
			{
				uint aliasNumber = config.readEntry( (*it) + "_id", 0 );
				QString aliasCommand = config.readEntry( (*it) + "_command", QString() );
				QStringList protocols = config.readEntry( (*it) + "_protocols", QStringList() );

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
	KConfigGroup config = KGlobal::config()->group("AliasPlugin");
	config.deleteGroup();

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

		config.writeEntry( item->text(0) + "_id", item->id );
		config.writeEntry( item->text(0) + "_command", item->text(1) );
		config.writeEntry( item->text(0) + "_protocols", protocols );

		item = (AliasItem*)item->nextSibling();
        }

	config.writeEntry( "AliasNames", aliases );
	config.sync();
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

	for( ProtocolList::ConstIterator it = p.begin(); it != p.end(); ++it )
	{
		Kopete::CommandHandler::commandHandler()->registerAlias(
			*it,
			alias,
			command,
			QString::fromLatin1("Custom alias for %1").arg(command),
			Kopete::CommandHandler::UserAlias
		);

		protocolMap.insert( QPair<Kopete::Protocol*,QString>( *it, alias ), true );
	}
}

void AliasPreferences::slotAddAlias()
{
	EditAliasDialog addDialog(this);
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
					"</qt>", alias),i18n("Invalid Alias Name") );
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
						"command is already being handled either by another alias or "
						"Kopete itself.</qt>", alias), i18n("Could Not Add Alias") );
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
	Q3ListViewItem *item = dialog->protocolList->firstChild();

	while( item )
	{
		if( item->isSelected() )
		{

			// If you don't have the selected protocol enabled, Kopete::PluginManager::self()->plugin
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
	foreach(const KPluginInfo &pluginInfo, Kopete::PluginManager::self()->availablePlugins("Protocols"))
	{
		ProtocolItem *item = new ProtocolItem( dialog->protocolList, pluginInfo );
		itemMap[ (Kopete::Protocol*)Kopete::PluginManager::self()->plugin( pluginInfo.pluginName() ) ] = item;
	}
}

void AliasPreferences::slotEditAlias()
{
	EditAliasDialog editDialog;

	loadProtocols( &editDialog );

	Q3ListViewItem *item = preferencesDialog->aliasList->selectedItems().first();
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
						"</qt>", alias),i18n("Invalid Alias Name") );
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
							"Kopete itself.</qt>", alias), i18n("Could Not Add Alias") );
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
        QList< Q3ListViewItem* > items = preferencesDialog->aliasList->selectedItems();
        if( items.isEmpty())
            return;
	if( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to delete the selected aliases?"), i18n("Delete Aliases"), KGuiItem(i18n("Delete"), "edit-delete") ) == KMessageBox::Continue )
	{
		foreach( Q3ListViewItem *i, items)
		{
			ProtocolList protocols = static_cast<AliasItem*>( i )->protocolList;
			for( ProtocolList::Iterator it = protocols.begin(); it != protocols.end(); ++it )
			{
				Kopete::CommandHandler::commandHandler()->unregisterAlias(
					*it,
					i->text(0)
				);

				protocolMap.remove( QPair<Kopete::Protocol*,QString>( *it, i->text(0) ) );
			}

			aliasMap.remove( i->text(0) );
			delete i;
			emit KCModule::changed(true);
		}

		save();
	}
        slotCheckAliasSelected();
}

void AliasPreferences::slotCheckAliasSelected()
{
	int numItems = preferencesDialog->aliasList->selectedItems().count();
	preferencesDialog->deleteButton->setEnabled( numItems > 0 );
	preferencesDialog->editButton->setEnabled( numItems  == 1 );
}

#include "aliaspreferences.moc"

