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

#ifndef HISTORYACTIONMANAGER_H
#define HISTORYACTIONMANAGE

#include "akonadihistoryplugin.h"

#include <KXMLGUIClient>

class KDialog;
class KLineEdit;
namespace Kopete { class ChatSession; }

class HistoryActionManager : public QObject , public KXMLGUIClient
{
    Q_OBJECT
public:
	
	HistoryActionManager(Kopete::ChatSession* parent = 0 , QObject* hPlugin = 0 );
	~HistoryActionManager();
	
	void processTag(QString&) ;
	
private:
	
	QPointer<AkonadiHistoryPlugin> m_hPlugin;
	Kopete::ChatSession * m_manager;
	QPointer<KLineEdit> m_lineEdit;
	QPointer<KDialog> m_getTagDialog;
	
private slots:
	void slotApplyClicked();
	void slotAddTag();
		
	
};

#endif // HISTORYACTIONMANAGER_H
