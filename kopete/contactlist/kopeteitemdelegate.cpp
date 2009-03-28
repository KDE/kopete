/*
    Kopete View Item Delegate

    Copyright (c) 2007 by Matt Rogers <mattr@kde.org>
    Copyright (c) 2009 by Roman Jarosz <kedgedev@gmail.com>

    Kopete    (c) 2002-2009 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteitemdelegate.h"
#include "kopeteitembase.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QApplication>

#include <qimageblitz.h>

#include <KIconLoader>

#include "kopetemetacontact.h"
#include "kopetecontact.h"
#include "kopeteappearancesettings.h"
#include "contactlistlayoutmanager.h"
#include "contactlistproxymodel.h"

const qreal MARGIN = 2.0;
const qreal MARGINH = 6.0;
const qreal PADDING = 1.0;

KopeteItemDelegate::KopeteItemDelegate( QAbstractItemView* parent )
: QStyledItemDelegate( parent )
{
}

KopeteItemDelegate::~KopeteItemDelegate()
{
}

QFont KopeteItemDelegate::normalFont( const QFont& naturalFont )
{
	if ( Kopete::AppearanceSettings::self()->contactListUseCustomFont() )
		return Kopete::AppearanceSettings::self()->contactListNormalFont();
	else
		return naturalFont;
}

QFont KopeteItemDelegate::smallFont( const QFont& naturalFont )
{
	if ( Kopete::AppearanceSettings::self()->contactListUseCustomFont() )
		return Kopete::AppearanceSettings::self()->contactListSmallFont();

	QFont font( naturalFont );
	if ( font.pixelSize() != -1 )
		font.setPixelSize( (font.pixelSize() * 3) / 4 );
	else
		font.setPointSizeF( font.pointSizeF() * 0.75 );
	return font;
}

QSize KopeteItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		QFont normal = normalFont( option.font );
		QFont small = smallFont( option.font );

		qreal height = 0;

		ContactList::ContactListLayout layout = ContactList::LayoutManager::instance()->activeLayout();
		int rowCount = layout.layout().rows();
		for ( int i = 0; i < rowCount; i++ )
			height += calculateRowHeight( layout.layout().row( i ), normal, small );

		height += MARGIN * 2 + ( rowCount - 1 ) * PADDING;
		return QSize( 120, height );
		
	}
	else
		return QStyledItemDelegate::sizeHint( option, index );
}

Kopete::Contact* KopeteItemDelegate::contactAt( const QStyleOptionViewItem& option, const QModelIndex& index, const QPoint& point ) const
{
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		ContactList::ContactListLayout layout = ContactList::LayoutManager::instance()->activeLayout();
		QList< QPair<QRect, Kopete::Contact*> > contactPositionList;
		paintItem( layout.layout(), 0, option, index, &contactPositionList );

		QPoint delegatePoint = point - option.rect.topLeft();
		for ( int i = 0; i < contactPositionList.size(); ++i )
		{
			if ( contactPositionList.at(i).first.contains( delegatePoint ) )
				return contactPositionList.at(i).second;
		}
	}
	return 0;
}

void KopeteItemDelegate::paint( QPainter* painter, 
								const QStyleOptionViewItem& option,
								const QModelIndex& index ) const
{
	//pull in contact settings: idleContactColor, greyIdleMetaContacts
	//pull in contact list settings: contactListDisplayMode
	QStyleOptionViewItem opt = option;
	
	if ( index.data( Kopete::Items::TypeRole ) == Kopete::Items::MetaContact )
	{
		//check the idle state of the metacontact and apply the appropriate color
		QVariant v = index.data( Kopete::Items::IdleTimeRole );
		if ( Kopete::AppearanceSettings::self()->greyIdleMetaContacts() && v.toInt() > 0 )
		{
			QColor idleColor( Kopete::AppearanceSettings::self()->idleContactColor() );
			opt.palette.setColor( QPalette::Text, idleColor );
		}

		ContactList::ContactListLayout layout = ContactList::LayoutManager::instance()->activeLayout();

		painter->save();

		// Draw background
		QApplication::style()->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter );

		painter->translate( option.rect.topLeft() );

		if ( option.state & QStyle::State_Selected )
			painter->setPen( option.palette.color( QPalette::Normal, QPalette::HighlightedText ) );
		else
			painter->setPen( option.palette.color( QPalette::Normal, QPalette::Text ) );

		paintItem( layout.layout(), painter, option, index, 0 );

		painter->restore();
	}
	else if (  index.data( Kopete::Items::TypeRole ) == Kopete::Items::Group )
	{
		QColor gc( Kopete::AppearanceSettings::self()->groupNameColor() );
		opt.palette.setColor( QPalette::Text, gc );
		QStyledItemDelegate::paint( painter, opt, index );
	}
	else
	{
		QStyledItemDelegate::paint( painter, opt, index );
	}
}

void KopeteItemDelegate::paintItem( ContactList::LayoutItemConfig config, QPainter* painter,
                                    const QStyleOptionViewItem& option, const QModelIndex& index,
                                    QList<QPair<QRect, Kopete::Contact*> >* contactPositionList ) const
{
	int rowCount = config.rows();
	if ( rowCount == 0 )
		return;

	const int hBorderMargin = MARGIN * 2;
	const int hMargins = hBorderMargin + ( rowCount - 1 ) * PADDING;

	int rowOffsetX = MARGIN;
	int rowOffsetY = MARGIN;

	if ( config.showIcon() )
	{
		int imageSize = option.rect.height() - hBorderMargin;

		if ( painter )
		{
			QRectF nominalImageRect( rowOffsetX, rowOffsetY, imageSize, imageSize );

			QVariant metaContactPicture;
			if ( index.data( Kopete::Items::HasNewMessageRole ).toBool() )
				metaContactPicture = QString::fromUtf8( "mail-unread" );
			else
				metaContactPicture = index.data( Kopete::Items::MetaContactImageRole );

			if ( metaContactPicture.type() == QVariant::Image )
			{
				// We have contact photo
				QImage metaContactImage = metaContactPicture.value<QImage>();
				if ( !metaContactImage.isNull() )
				{
					metaContactImage = metaContactImage.scaled( imageSize, imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation );

					int metaContactStatus = index.data( Kopete::Items::OnlineStatusRole ).toInt();
					if ( metaContactStatus == Kopete::OnlineStatus::Offline )
						Blitz::grayscale( metaContactImage );

					switch ( metaContactStatus )
					{
					case Kopete::OnlineStatus::Online:
						break;
					case Kopete::OnlineStatus::Away:
						Blitz::fade( metaContactImage, 0.5, Qt::white );
						break;
					case Kopete::OnlineStatus::Offline:
						Blitz::fade( metaContactImage, 0.4, Qt::white );
						break;
					case Kopete::OnlineStatus::Unknown:
					default:
						Blitz::fade( metaContactImage, 0.8, Qt::white );
					}

					QPixmap photoPixmap;
					bool roundedIcons = Kopete::AppearanceSettings::self()->contactListIconRounded();
					if ( roundedIcons )
					{
						photoPixmap = QPixmap( metaContactImage.width(), metaContactImage.height() );
						photoPixmap.fill( Qt::transparent );
						QPainter p( &photoPixmap );
						p.setRenderHint( QPainter::Antialiasing );
						p.setPen( Qt::NoPen );
						p.setBrush( QBrush( metaContactImage ) );
						QRectF rectangle( 0.5, 0.5, photoPixmap.width()-1, photoPixmap.height()-1 );
						p.drawRoundedRect( rectangle, 25, 25, Qt::RelativeSize );
					}
					else
					{
						photoPixmap = QPixmap::fromImage( metaContactImage );
					}

					if ( Kopete::AppearanceSettings::self()->contactListIconBorders() )
					{
						QPainter p( &photoPixmap );
						p.setPen( Qt::black );

						if ( roundedIcons )
						{
							p.setRenderHint( QPainter::Antialiasing );
							QRectF rectangle( 0.5, 0.5, photoPixmap.width()-1, photoPixmap.height()-1 );
							p.drawRoundedRect( rectangle, 25, 25, Qt::RelativeSize );
						}
						else
						{
							p.drawRect( 0, 0, photoPixmap.width()-1, photoPixmap.height()-1 );
						}
					}
					//offset cover if non square
					QPointF offset = centerImage( photoPixmap, nominalImageRect );
					QRectF imageRect( nominalImageRect.x() + offset.x(),
									nominalImageRect.y() + offset.y(),
									nominalImageRect.width() - offset.x() * 2,
									nominalImageRect.height() - offset.y() * 2 );

					painter->drawPixmap( imageRect, photoPixmap, QRectF( photoPixmap.rect() ) );
				}
			}
			else
			{
				// We have icon
				QString metaContactImageName = metaContactPicture.value<QString>();
				QPixmap metaContactImage = SmallIcon( metaContactImageName, imageSize );
				if ( !metaContactImage.isNull() )
				{
					//offset cover if non square
					QPointF offset = centerImage( metaContactImage, nominalImageRect );
					QRectF imageRect( nominalImageRect.x() + offset.x(),
									nominalImageRect.y() + offset.y(),
									nominalImageRect.width() - offset.x() * 2,
									nominalImageRect.height() - offset.y() * 2 );

					painter->drawPixmap( imageRect, metaContactImage, QRectF( metaContactImage.rect() ) );
				}
			}
		}

		rowOffsetX += imageSize + MARGINH;
	}

	QFont normal = normalFont( option.font );
	QFont small = smallFont( option.font );

	for ( int i = 0; i < rowCount; i++ )
	{
		ContactList::LayoutItemConfigRow row = config.row( i );
		const int rowHeight = calculateRowHeight( row, normal, small );
		qreal itemOffsetX = rowOffsetX;
		const int elementCount = row.count();
		qreal rowWidth = option.rect.width() - ( rowOffsetX + MARGINH );

		QRectF rowBox( itemOffsetX, rowOffsetY, rowWidth, rowHeight );
		int currentItemX = itemOffsetX;

		//we need to do a quick pass to figure out how much space is left for auto sizing elements
		qreal spareSpace = 1.0;
		int autoSizeElemCount = 0;
		for ( int k = 0; k < elementCount; ++k )
		{
			spareSpace -= row.element( k ).size();
			if ( row.element( k ).size() < 0.001 )
				autoSizeElemCount++;
		}

		qreal spacePerAutoSizeElem = spareSpace / (qreal) autoSizeElemCount;
		for ( int j = 0; j < elementCount; ++j )
		{
			ContactList::LayoutItemConfigRowElement element = row.element( j );

			const int value = element.value();
			const int role = ContactList::LayoutManager::instance()->token( value ).mModelRole;

			qreal itemWidth = 0.0;

			int alignment = element.alignment();

			QFont font( ( element.small() ) ? small : normal );
			font.setBold( element.bold() );
			font.setItalic( element.italic() );
			if ( painter )
				painter->setFont( font );

			QRectF elementBox;

			qreal size;
			if ( element.size() > 0.0001 )
				size = element.size();
			else
				size = spacePerAutoSizeElem;

			if ( size > 0.0001 )
			{
				itemWidth = rowWidth * size;

				//special case for painting the ProtocolIcons...
				if ( value == ContactList::LayoutManager::ProtocolIcons )
				{
					QObject* metaContactObject = qVariantValue<QObject*>( index.data( Kopete::Items::ObjectRole ) );
					Kopete::MetaContact* metaContact = qobject_cast<Kopete::MetaContact*>(metaContactObject);

					QList<Kopete::Contact*> contactList = metaContact->contacts();
					if ( contactList.size() > 0 )
					{
						const qreal IconMarginH = 2.0;
						const qreal IconMarginV = 1.0;
						const qreal IconSize = rowHeight - 2 * IconMarginV;
						qreal iconsWidth = contactList.size() * IconSize;
						if ( contactList.size() > 1 )
							iconsWidth += (contactList.size() - 1) * IconMarginH;

						if (iconsWidth > itemWidth)
							iconsWidth = itemWidth;

						QRectF drawingRect( currentItemX, rowOffsetY + IconMarginV, iconsWidth, IconSize );
						if ( (alignment & Qt::AlignRight) == Qt::AlignRight )
							drawingRect.moveRight( currentItemX + itemWidth );
						else if ( (alignment & Qt::AlignHCenter) == Qt::AlignHCenter )
							drawingRect.moveLeft( currentItemX + (itemWidth - iconsWidth) / 2.0 );

						double offsetX = 0;
						foreach ( Kopete::Contact *contact, contactList )
						{
							QIcon contactIcon = contact->onlineStatus().iconFor( contact );
							if ( contactIcon.isNull() )
								continue;

							QRectF pixmapRect( drawingRect.x() + offsetX, drawingRect.y(),
							                   IconSize, IconSize );

							if ( contactPositionList )
								contactPositionList->append( QPair<QRect, Kopete::Contact*>( pixmapRect.toRect(), contact ) );

							if ( painter )
							{
								QPixmap contactPixmap = contactIcon.pixmap( IconSize, IconSize );
								painter->drawPixmap( pixmapRect, contactPixmap, QRectF( contactPixmap.rect() ) );
							}

							offsetX += IconSize + IconMarginH;
						}
					}
				}
				else
				{
					if ( painter )
					{
						QString text = ( role > -1 ) ? index.data( role ).toString() : QString();
						text = element.prefix() + text + element.suffix();
						text = QFontMetricsF( font ).elidedText( text, Qt::ElideRight, itemWidth );
						painter->drawText( currentItemX, rowOffsetY, itemWidth, rowHeight, alignment, text );
					}
				}

				currentItemX += itemWidth;
			}
		}
		rowOffsetY += rowHeight + PADDING;
	}
}

QPointF KopeteItemDelegate::centerImage( const QImage& image, const QRectF& rect ) const
{
	qreal imageRatio = ( qreal )image.width() / ( qreal )image.height();

	qreal moveByX = 0.0;
	qreal moveByY = 0.0;

	if ( imageRatio >= 1 )
		moveByY = ( rect.height() - ( rect.width() / imageRatio ) ) / 2.0;
	else
		moveByX = ( rect.width() - ( rect.height() * imageRatio ) ) / 2.0;

	return QPointF( moveByX, moveByY );
}

QPointF KopeteItemDelegate::centerImage( const QPixmap& pixmap, const QRectF& rect ) const
{
	qreal pixmapRatio = ( qreal )pixmap.width() / ( qreal )pixmap.height();
	
	qreal moveByX = 0.0;
	qreal moveByY = 0.0;
	
	if ( pixmapRatio >= 1 )
		moveByY = ( rect.height() - ( rect.width() / pixmapRatio ) ) / 2.0;
	else
		moveByX = ( rect.width() - ( rect.height() * pixmapRatio ) ) / 2.0;
	
	return QPointF( moveByX, moveByY );
}

qreal KopeteItemDelegate::calculateRowHeight( const ContactList::LayoutItemConfigRow &row, const QFont &normal, const QFont &small ) const
{
	qreal rowHeight = 0;

	const int elementCount = row.count();
	for ( int i = 0; i < elementCount; ++i )
	{
		ContactList::LayoutItemConfigRowElement element = row.element( i );
		QFont elementFont( ( element.small() ) ? small : normal );
		elementFont.setItalic( element.italic() );
		elementFont.setBold( element.bold() );
		rowHeight = qMax( rowHeight, QFontMetricsF( elementFont ).height() );
	}
	return rowHeight;
}
