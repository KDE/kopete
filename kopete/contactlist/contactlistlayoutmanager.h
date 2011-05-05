/***************************************************************************
 *   Copyright (c) 2008  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
 *   Copyright (c) 2009  Roman Jarosz         <kedgedev@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
 
#ifndef CONTACTLISTLAYOUTMANAGER_H
#define CONTACTLISTLAYOUTMANAGER_H

#include <QStringList>
#include <QString>
#include <QMap>

#include <kopete_export.h>

#include "contactlistlayoutitemconfig.h"

class QDomElement;
class QDomDocument;

namespace ContactList {

class KOPETE_CONTACT_LIST_EXPORT ContactListTokenConfig {
public:
	ContactListTokenConfig()
		: mModelRole(-1)
	{}
	
	ContactListTokenConfig(int modelRole, QString configName, QString name, QString iconName)
		: mModelRole(modelRole), mConfigName(configName), mName(name), mIconName(iconName)
	{}
	
	int mModelRole;
	QString mConfigName;
	QString mName;
	QString mIconName;
};

class KOPETE_CONTACT_LIST_EXPORT LayoutManager : public QObject
{
	Q_OBJECT

public:
	static LayoutManager * instance();

	QStringList layouts() const;
	void setActiveLayout( const QString &layout );
	void setPreviewLayout( const ContactListLayout &layout );
	ContactListLayout layout( const QString &layout );
	ContactListLayout activeLayout();
	QString activeLayoutName() const;

	bool isDefaultLayout( const QString &layout ) const;

	bool addUserLayout( const QString &name, ContactListLayout layout );
	bool deleteLayout( const QString &layout );

	enum TokenTypes {
		PlaceHolder = 0,
		DisplayName = 1,
		StatusTitle = 2,
		StatusMessage = 3,
		ContactIcons = 4
	};

	QList<ContactListTokenConfig> tokens() const { return m_tokens; }
	ContactListTokenConfig token( int tokenType ) const { return m_tokens.value( tokenType ); }

signals:
	void activeLayoutChanged();
	void layoutListChanged();

private:
	LayoutManager();
	~LayoutManager();

	void loadDefaultLayouts();
	void loadUserLayouts();

	void loadLayouts( const QString &fileName, bool user );

	QDomElement createItemElement( QDomDocument doc, const QString &name,  const LayoutItemConfig &item ) const;

	LayoutItemConfig parseItemConfig( const QDomElement &elem );

	static LayoutManager * s_instance;

	QMap<QString, ContactListLayout> m_layouts;
	QString m_activeLayout;
	ContactListLayout m_previewLayout;
	QList<ContactListTokenConfig> m_tokens;
};

}

#endif
