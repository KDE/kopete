/***************************************************************************
 *   Copyright (c) 2009  Nikolaj Hald Nielsen <nhnFreespirit@gmail.com>    *
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

#include "contactlistlayouteditwidget.h"
#include "DragStack.h"
#include "contactlistlayoutmanager.h"
#include "contactlisttoken.h"

#include <KHBox>
#include <KLocale>
#include <KDebug>

#include <QCheckBox>
#include <QSpinBox>

using namespace ContactList;

LayoutEditWidget::LayoutEditWidget( QWidget *parent )
: KVBox(parent)
{
	m_tokenFactory = new ContactListTokenFactory;
	m_dragstack = new DragStack( "application/x-kopete-contactlist-token", this );
	m_dragstack->setCustomTokenFactory( m_tokenFactory );
	connect( m_dragstack, SIGNAL(focussed(QWidget*)), this, SIGNAL(focussed(QWidget*)) );
	connect( m_dragstack, SIGNAL(changed()), this, SIGNAL(changed()) );
	
	m_showIconCheckBox = new QCheckBox( i18n( "Show Icon" ) , this );
}


LayoutEditWidget::~LayoutEditWidget()
{
//     delete m_tokenFactory; m_tokenFactory = 0;
}

void LayoutEditWidget::readLayout( ContactList::LayoutItemConfig config )
{
	int rowCount = config.rows();

	m_showIconCheckBox->setChecked( config.showIcon() );

	m_dragstack->clear();

	for( int i = 0; i < rowCount; i++ )
	{
		//get the row config
		ContactList::LayoutItemConfigRow rowConfig = config.row( i );

		int elementCount = rowConfig.count();

		//FIXME! for now, each element get the same size. This needs extensions to the token stuff
		//qreal size = 1.0 / (qreal) elementCount;
		
		for( int j = 0; j < elementCount; j++ )
		{
			ContactList::LayoutItemConfigRowElement element = rowConfig.element( j );
			ContactList::ContactListTokenConfig clToken = ContactList::LayoutManager::instance()->token( element.value() );
			ContactListToken *token =  new ContactListToken( clToken.mName, clToken.mIconName, element.value(), m_dragstack );
			token->setBold( element.bold() );
			token->setSmall( element.small() );
			token->setItalic( element.italic() );
			token->setAlignment( element.alignment() );
			m_dragstack->insertToken( token, i, j );
			token->setWidth( element.size() * 100.0 );
		}

	}
}

ContactList::LayoutItemConfig LayoutEditWidget::config()
{

	LayoutItemConfig config;
	config.setShowIcon( m_showIconCheckBox->isChecked() );
	
	int noOfRows = m_dragstack->rows();

	for( int i = 0; i < noOfRows; i++ )
	{

		LayoutItemConfigRow currentRowConfig;

		QList<Token *> tokens = m_dragstack->drags( i );

		foreach( Token * token, tokens ) {
			if ( ContactListToken *twl = dynamic_cast<ContactListToken *>( token ) )
			{
				qreal width = 0.0;
				if ( twl->widthForced() && twl->width() > 0.01) {
					width = twl->width();
				}
				currentRowConfig.addElement( LayoutItemConfigRowElement( twl->value(), width, twl->bold(), twl->italic(), twl->small(),
				                                                         twl->alignment(), twl->prefix(), twl->suffix() ) );
			}
		}

		config.addRow( currentRowConfig );

	}
	return config;
}


#include "contactlistlayouteditwidget.moc"

