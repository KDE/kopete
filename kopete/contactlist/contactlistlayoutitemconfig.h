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

#ifndef CONTACTLISTLAYOUTITEMCONFIG_H
#define CONTACTLISTLAYOUTITEMCONFIG_H

#include <QList>
#include <QString>

#include <kopete_export.h>

namespace ContactList {

class KOPETE_CONTACT_LIST_EXPORT LayoutItemConfigRowElement
{
	public:
		LayoutItemConfigRowElement( int value, qreal size, bool bold, bool italic, bool small,
		                            Qt::Alignment alignment, const QString &prefix = QString(),
		                            const QString &suffix = QString() );

		int value() const;
		qreal size() const;
		bool bold() const;
		bool italic() const;
		Qt::Alignment alignment() const;
		QString prefix() const;
		QString suffix() const;
		bool small() const { return m_small; }
	
	private:
		int m_value;
		qreal m_size;
		bool m_bold;
		bool m_italic;
		bool m_small;
		Qt::Alignment m_alignment;
		QString m_prefix, m_suffix;
};

class KOPETE_CONTACT_LIST_EXPORT LayoutItemConfigRow
{
	public:
		void addElement( LayoutItemConfigRowElement element );
		int count() const;
		LayoutItemConfigRowElement element( int at ) const;
	private:
		QList<LayoutItemConfigRowElement> m_elements;
};

/**
	This class wraps the data needed to paint a LayoutItemDelegate. It knows how many vertical
	rows there should be, how many items in each row, whether an image should be displayed and so on.
*/
class KOPETE_CONTACT_LIST_EXPORT LayoutItemConfig
{
	public:
		LayoutItemConfig();
		~LayoutItemConfig();

		int rows() const;
		LayoutItemConfigRow row( int at ) const;
		bool showIcon() const;

		void addRow( LayoutItemConfigRow row );
		void setShowIcon( bool showIcon );
	
	private:
		QList<LayoutItemConfigRow> m_rows;
		bool m_showIcon;
};


class KOPETE_CONTACT_LIST_EXPORT ContactListLayout
{
	public:
		LayoutItemConfig layout() const;
		bool isEditable() const;

		void setLayout( LayoutItemConfig layout );
		void setIsEditable( bool editable );

	private:
		LayoutItemConfig m_layout;
		bool m_isEditable;
};

}

#endif
