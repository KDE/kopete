/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "historyactionmanager.h"
#include "akonadihistoryplugin.h"

#include <kgenericfactory.h>
#include <KAction>
#include <KActionCollection>
#include <KLineEdit>

#include "kopetechatsession.h"
#include <KDialog>

HistoryActionManager::HistoryActionManager(Kopete::ChatSession* parent, QObject* hPlugin)
      : QObject(parent) ,KXMLGUIClient(parent) , m_manager(parent) 
{
	kDebug() << "" ;
	m_hPlugin = qobject_cast<AkonadiHistoryPlugin*>(hPlugin);
	
	setComponentData( m_hPlugin->xmlGuiInstance() );

	// Refuse to build this client, it is based on wrong parameters
	if ( !m_manager || m_manager->members().isEmpty() )
		deleteLater();
	
	kDebug() << "setting up actions";
	
	KAction *applytag = new KAction( KIcon("view-history"), i18n("Apply &Label" ), this);
	actionCollection()->addAction("tagThisChatHistory", applytag);
	applytag->setShortcut(KShortcut (Qt::CTRL + Qt::Key_L));

	connect( applytag, SIGNAL(triggered(bool)), this, SLOT(slotAddTag()) );
	
	setXMLFile ( "tagchatui.rc" );
	
}

HistoryActionManager::~HistoryActionManager()
{

}

void HistoryActionManager::slotAddTag()
{
	kDebug() << "Slot add Tag called :) ";
	
	m_lineEdit = new KLineEdit() ;
	
	m_getTagDialog = new KDialog( );
	m_getTagDialog->setCaption( "Enter Label" );
	m_getTagDialog->setButtons( KDialog::Apply | KDialog::Cancel );
	m_getTagDialog->setDefaultButton(KDialog::Apply);
	m_getTagDialog->setMainWidget(m_lineEdit);

	connect(m_getTagDialog, SIGNAL(applyClicked()), this , SLOT(slotApplyClicked()) );
	m_getTagDialog->show();
	
}

void HistoryActionManager::slotApplyClicked()
{
	QString tag = m_lineEdit->userText() ;

	m_lineEdit->deleteLater();
	m_getTagDialog->deleteLater();
	
	processTag(tag);
}


void HistoryActionManager::processTag(QString& tagString)
{
	kDebug() << " ";
	kDebug() << tagString ;
}



