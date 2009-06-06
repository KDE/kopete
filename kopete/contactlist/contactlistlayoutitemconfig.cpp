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

#include "contactlistlayoutitemconfig.h"

namespace ContactList {

LayoutItemConfigRowElement::LayoutItemConfigRowElement( int value, qreal size, bool bold, bool italic, bool small, bool optimalSize,
                                                        Qt::Alignment alignment, const QString &prefix, const QString &suffix )
	: m_value( value )
	, m_size( size )
	, m_bold( bold )
	, m_italic( italic )
	, m_small( small )
	, m_optimalSize( optimalSize )
	, m_alignment( alignment )
	, m_prefix( prefix )
	, m_suffix( suffix )
{
}

//////////////////////////////////////////////

void LayoutItemConfigRow::addElement( LayoutItemConfigRowElement element )
{
	m_elements.append( element );
}

int LayoutItemConfigRow::count() const
{
	return m_elements.count();
}

LayoutItemConfigRowElement LayoutItemConfigRow::element( int at ) const
{
	return m_elements.at( at );
}


//////////////////////////////////////////////

LayoutItemConfig::LayoutItemConfig()
	: m_showIcon( false )
{
}

LayoutItemConfig::~LayoutItemConfig()
{
}

int LayoutItemConfig::rows() const
{
	return m_rows.size();
}


LayoutItemConfigRow ContactList::LayoutItemConfig::row( int at ) const
{
	return m_rows.at( at );
}

void ContactList::LayoutItemConfig::addRow( LayoutItemConfigRow row )
{
	m_rows.append( row );
}

bool LayoutItemConfig::showIcon() const
{
	return m_showIcon;
}

void ContactList::LayoutItemConfig::setShowIcon( bool showIcon )
{
	m_showIcon = showIcon;
}

//////////////////////////////////////////////


LayoutItemConfig ContactListLayout::layout() const
{
	return m_layout;
}

void ContactListLayout::setLayout( LayoutItemConfig layout )
{
	m_layout = layout;
}

bool ContactListLayout::isEditable() const
{
	return m_isEditable;
}

void ContactListLayout::setIsEditable( bool editable )
{
	m_isEditable = editable;
}

}
