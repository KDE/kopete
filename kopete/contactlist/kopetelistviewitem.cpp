/*
    kopetelistviewitem.cpp - Kopete's modular QListViewItems

    Copyright (c) 2004      by Richard Smith          <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "kopetelistviewitem.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include <qpixmap.h>
#include <qpainter.h>
#include <qptrlist.h>
#include <qrect.h>
#include <qtimer.h>
#include <qheader.h>

#ifdef HAVE_XRENDER
#  include <X11/Xlib.h>
#  include <X11/extensions/Xrender.h>
#endif

namespace Kopete {
namespace UI {
namespace ListView {

// ComponentBase --------

class ComponentBase::Private
{
public:
	QPtrList<Component> components;
};

ComponentBase::ComponentBase()
 : d( new Private )
{
}

ComponentBase::~ComponentBase()
{
	d->components.setAutoDelete( true );
	delete d;
}

uint ComponentBase::components() { return d->components.count(); }
Component *ComponentBase::component( uint n ) { return d->components.at( n ); }

Component *ComponentBase::componentAt( const QPoint &pt )
{
	for ( uint n = 0; n < components(); ++n )
	{
		if ( component( n )->rect().contains( pt ) )
		{
			if ( Component *comp = component( n )->componentAt( pt ) )
				return comp;
			return component( n );
		}
	}
	return 0;
}

void ComponentBase::componentAdded( Component *component )
{
	d->components.append( component );
}

void ComponentBase::componentRemoved( Component *component )
{
	//TODO: make sure the component is in d->components once and only once.
	// if not, the situation is best referred to as 'very very broken indeed'.
	d->components.remove( component );
}

void ComponentBase::componentResized( Component * )
{
}

void ComponentBase::updateAnimationPosition( int p, int s )
{
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		QRect start = comp->startRect();
		QRect target = comp->targetRect();
		QRect rc( start.left() + ((target.left() - start.left()) * p) / s,
		          start.top() + ((target.top() - start.top()) * p) / s,
		          start.width() + ((target.width() - start.width()) * p) / s,
		          start.height() + ((target.height() - start.height()) * p) / s );
		comp->setRect( rc );
		comp->updateAnimationPosition( p, s );
	}
}

// Component --------

class Component::Private
{
public:
	Private( ComponentBase *parent )
	 : parent( parent ), minWidth( 0 ), minHeight( 0 )
	 , growHoriz( false ), growVert( false )
	{
	}
	ComponentBase *parent;
	QRect rect;
	QRect startRect, targetRect;
	int minWidth, minHeight;
	bool growHoriz, growVert;
};

Component::Component( ComponentBase *parent )
 : d( new Private( parent ) )
{
	d->parent->componentAdded( this );
}

Component::~Component()
{
	d->parent->componentRemoved( this );
	delete d;
}

QRect Component::rect() { return d->rect; }
QRect Component::startRect() { return d->startRect; }
QRect Component::targetRect() { return d->targetRect; }

int Component::minWidth() { return d->minWidth; }
int Component::minHeight() { return d->minHeight; }
int Component::widthForHeight( int ) { return minWidth(); }
int Component::heightForWidth( int ) { return minHeight(); }
bool Component::stretchHoriz() { return d->growHoriz; }
bool Component::stretchVert() { return d->growVert; }

bool Component::setMinWidth( int width, bool canUseMore )
{
	if ( d->minWidth == width && d->growHoriz == canUseMore ) return false;
	d->minWidth = width;
	d->growHoriz = canUseMore;
	
	d->parent->componentResized( this );
	return true;
}
bool Component::setMinHeight( int height, bool canUseMore )
{
	if ( d->minHeight == height && d->growVert == canUseMore ) return false;
	d->minHeight = height;
	d->growVert = canUseMore;

	d->parent->componentResized( this );
	return true;
}

void Component::layout( const QRect &newRect )
{
	if ( rect().isNull() )
		d->startRect = QRect( newRect.topLeft(), newRect.topLeft() );
	else
		d->startRect = rect();
	d->targetRect = newRect;
	//kdDebug(14000) << k_funcinfo << "At " << rect << endl;
}

void Component::setRect( const QRect &rect )
{
	d->rect = rect;
}

void Component::paint( QPainter *painter, const QColorGroup &cg )
{
	/*painter->setPen( Qt::red );
	painter->drawRect( rect() );*/
	for ( uint n = 0; n < components(); ++n )
		component( n )->paint( painter, cg );
}

void Component::repaint()
{
	d->parent->repaint();
}

void Component::relayout()
{
	d->parent->relayout();
}

void Component::componentAdded( Component *component )
{
	ComponentBase::componentAdded( component );
	//update( Relayout );
}

void Component::componentRemoved( Component *component )
{
	ComponentBase::componentRemoved( component );
	//update( Relayout );
}

// BoxComponent --------

class BoxComponent::Private
{
public:
	Private( BoxComponent::Direction dir ) : direction( dir ) {}
	BoxComponent::Direction direction;

	static const int padding = 2;
};

BoxComponent::BoxComponent( ComponentBase *parent, Direction dir )
 : Component( parent ), d( new Private( dir ) )
{
}
BoxComponent::~BoxComponent()
{
	delete d;
}

int BoxComponent::widthForHeight( int height )
{
	if ( d->direction != Horizontal )
		return Component::widthForHeight( height );

	int width = (components() - 1) * Private::padding;
	for ( uint n = 0; n < components(); ++n )
		width += component( n )->widthForHeight( height );
	return width;
}

int BoxComponent::heightForWidth( int width )
{
	if ( d->direction == Horizontal )
		return Component::heightForWidth( width );

	int height = (components() - 1) * Private::padding;
	for ( uint n = 0; n < components(); ++n )
		height += component( n )->heightForWidth( width );
	return height;
}

void BoxComponent::calcMinSize()
{
	int sum = (components() - 1) * Private::padding, max = 0;
	bool stretchH = false, stretchV = false;
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		if ( d->direction == Horizontal )
		{
			max = QMAX( max, comp->minHeight() );
			sum += comp->minWidth();
		}
		else
		{
			max = QMAX( max, comp->minWidth() );
			sum += comp->minHeight();
		}
		if ( comp->stretchHoriz() ) stretchH = true;
		if ( comp->stretchVert() ) stretchV = true;
	}

	bool sizeChanged = false;
	if ( d->direction == Horizontal )
	{
		if ( setMinWidth( sum, stretchH ) ) sizeChanged = true;
		if ( setMinHeight( max, stretchV ) ) sizeChanged = true;
	}
	else
	{
		if ( setMinWidth( max, stretchH ) ) sizeChanged = true;
		if ( setMinHeight( sum, stretchV ) ) sizeChanged = true;
	}

	if ( sizeChanged )
		repaint();
	else
		relayout();
}

void BoxComponent::layout( const QRect &rect )
{
	Component::layout( rect );

	bool horiz = (d->direction == Horizontal);
	int fixedSize = 0;
	int numVariable = 0;
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		if ( horiz )
		{
			if ( comp->stretchHoriz() )
				numVariable++;
			fixedSize += comp->minWidth();
		}
		else
		{
			if ( comp->stretchVert() )
				numVariable++;
			fixedSize += comp->minHeight();
		}
	}
	int numFixed = components() - numVariable;

	// remaining space after all fixed items have been allocated
	int padding = Private::padding;

	// ensure total is at least minXXX. the only time the rect
	// will be smaller than that is when we don't fit, and in
	// that cases we should pretend that we're wide/high enough.
	int total;
	if ( horiz )
		total = QMAX( rect.width(), minWidth() );
	else
		total = QMAX( rect.height(), minHeight() );

	int remaining = total - fixedSize - padding * (components() - 1);

	// extra space for each variable-size and each fixed-size item
	int eachVariable = 0;
	if ( numVariable > 0 )
		eachVariable = remaining / numVariable;
	else if ( numFixed > 1 )
		padding += remaining / ( numFixed - 1 );

	// finally, lay everything out
	int pos = 0;
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		QRect rc;
		if ( horiz )
		{
			rc.setLeft( rect.left() + pos );
			rc.setTop( rect.top() );
			rc.setHeight( rect.height() );
			if ( comp->stretchHoriz() )
				rc.setWidth( comp->minWidth() + eachVariable );
			else
				rc.setWidth( comp->minWidth() );
			pos += rc.width();
		}
		else
		{
			rc.setLeft( rect.left() );
			rc.setTop( rect.top() + pos );
			rc.setWidth( rect.width() );
			if ( comp->stretchVert() )
				rc.setHeight( comp->minHeight() + eachVariable );
			else
				rc.setHeight( comp->minHeight() );
			pos += rc.height();
		}
		comp->layout( rc & rect );
		pos += padding;
	}
}

void BoxComponent::componentAdded( Component *component )
{
	Component::componentAdded( component );
	calcMinSize();
}

void BoxComponent::componentRemoved( Component *component )
{
	Component::componentRemoved( component );
	calcMinSize();
}

void BoxComponent::componentResized( Component *component )
{
	Component::componentResized( component );
	calcMinSize();
}

// ImageComponent --------

class ImageComponent::Private
{
public:
	QPixmap image;
};

ImageComponent::ImageComponent( ComponentBase *parent )
 : Component( parent ), d( new Private )
{
}

ImageComponent::~ImageComponent()
{
	delete d;
}

void ImageComponent::setPixmap( const QPixmap &img )
{
	d->image = img;
	setMinWidth( img.width() );
	setMinHeight( img.height() );
	repaint();
}

static QPoint operator+( const QPoint &pt, const QSize &sz )
{
	return QPoint( pt.x() + sz.width(), pt.y() + sz.height() );
}

/*static QPoint operator+( const QSize &sz, const QPoint &pt )
{
	return pt + sz;
}*/

void ImageComponent::paint( QPainter *painter, const QColorGroup & )
{
	QRect ourRc = rect();
	QRect rc = d->image.rect();
	// center rc within our rect
	rc.moveTopLeft( ourRc.topLeft() + (ourRc.size() - rc.size()) / 2 );
	// paint, shrunk to be within our rect
	painter->drawPixmap( rc & ourRc, d->image );
}

// TextComponent

class TextComponent::Private
{
public:
	Private() : fixedWidth( false ), customColor( false ) {}
	QString text;
	bool fixedWidth;
	bool customColor;
	QColor color;
	QFont font;
};

TextComponent::TextComponent( ComponentBase *parent, const QFont &font, const QString &text, bool fixedWidth )
 : Component( parent ), d( new Private )
{
	setFont( font );
	setText( text );
	setFixedWidth( fixedWidth );
}

TextComponent::~TextComponent()
{
	delete d;
}

QString TextComponent::text()
{
	return d->text;
}

void TextComponent::setText( const QString &text )
{
	if ( text == d->text ) return;
	d->text = text;
	calcMinSize();
}

QFont TextComponent::font()
{
	return d->font;
}

void TextComponent::setFont( const QFont &font )
{
	if ( font == d->font ) return;
	d->font = font;
	calcMinSize();
}

bool TextComponent::fixedWidth()
{
	return d->fixedWidth;
}

void TextComponent::setFixedWidth( bool fixedWidth )
{
	if ( fixedWidth == d->fixedWidth ) return;
	d->fixedWidth = fixedWidth;
	calcMinSize();
}

void TextComponent::calcMinSize()
{
	if ( d->fixedWidth )
		setMinWidth( QFontMetrics( font() ).width( d->text ) + 2 );
	else
		setMinWidth( 0, true );

	if ( !d->text.isEmpty() )
		setMinHeight( QFontMetrics( font() ).height() );
	else
		setMinHeight( 0 );

	repaint();
}

QColor TextComponent::color()
{
	return d->color;
}

void TextComponent::setColor( const QColor &color )
{
	d->color = color;
	d->customColor = true;
	repaint();
}

void TextComponent::setDefaultColor()
{
	d->customColor = false;
	repaint();
}

void TextComponent::paint( QPainter *painter, const QColorGroup &cg )
{
	if ( d->customColor )
		painter->setPen( d->color );
	else
		painter->setPen( cg.text() );
	QString dispStr = KStringHandler::rPixelSqueeze( d->text, QFontMetrics( font() ), rect().width() );
	painter->setFont( font() );
	painter->drawText( rect(), Qt::SingleLine, dispStr );
}

// HSpacerComponent --------

HSpacerComponent::HSpacerComponent( ComponentBase *parent )
 : Component( parent )
{
	setMinWidth( 0, true );
	setMinHeight( 0 );
}

// VSpacerComponent --------

VSpacerComponent::VSpacerComponent( ComponentBase *parent )
 : Component( parent )
{
	setMinWidth( 0 );
	setMinHeight( 0, true );
}

// Item --------

class Item::Private
{
public:
	Private() : animateLayout( true ), opacity( 1.0 ), visibilityLevel( 0 ), visibilityTarget( false ) {}

	QTimer layoutTimer;

	QTimer layoutAnimateTimer;
	bool animateLayout;
	int layoutAnimateSteps;
	static const int layoutAnimateStepsTotal = 10;

	float opacity;
	QTimer visibilityTimer;
	int visibilityLevel;
	bool visibilityTarget;
	static const int visibilityFoldSteps = 7;
#ifdef HAVE_XRENDER
	static const int visibilityFadeSteps = 7;
#else
	static const int visibilityFadeSteps = 0;
#endif
	static const int visibilityStepsTotal = visibilityFoldSteps + visibilityFadeSteps;
};

Item::Item( QListViewItem *parent, QObject *owner, const char *name )
 : QObject( owner, name ), KListViewItem( parent ), d( new Private )
{
	initLVI();
}

Item::Item( QListView *parent, QObject *owner, const char *name )
 : QObject( owner, name ), KListViewItem( parent ), d( new Private )
{
	initLVI();
}

Item::~Item()
{
	delete d;
}

void Item::initLVI()
{
	connect( listView()->header(), SIGNAL( sizeChange( int, int, int ) ), SLOT( slotColumnResized() ) );
	connect( &d->layoutTimer, SIGNAL( timeout() ), SLOT( slotLayoutItems() ) );
	connect( &d->layoutAnimateTimer, SIGNAL( timeout() ), SLOT( slotLayoutAnimateItems() ) );
	connect( &d->visibilityTimer, SIGNAL( timeout() ), SLOT( slotUpdateVisibility() ) );
	setVisible( false );
	setTargetVisibility( true );
}

void Item::slotColumnResized()
{
	scheduleLayout();
	// if we've been resized, don't animate the layout
	d->animateLayout = false;
}

void Item::scheduleLayout()
{
	// perform a delayed layout in order to speed it all up
	if ( ! d->layoutTimer.isActive() )
		d->layoutTimer.start( 30, true );
}

void Item::slotLayoutItems()
{
	d->layoutTimer.stop();

	for ( uint n = 0; n < components(); ++n )
	{
		int width = listView()->columnWidth(n);
		if ( n == 0 )
		{
			int d = depth() + (listView()->rootIsDecorated() ? 1 : 0);
			width -= d * listView()->treeStepSize();
		}

		int height = component( n )->heightForWidth( width );
		component( n )->layout( QRect( 0, 0, width, height ) );
		//kdDebug(14000) << k_funcinfo << "Component " << n << " is " << width << " x " << height << endl;
	}

	setHeight(0);
	repaint();

	if ( !d->layoutAnimateTimer.isActive() )
		d->layoutAnimateTimer.start( 10 );
	d->layoutAnimateSteps = -1;
	if ( !d->animateLayout )
	{
		d->layoutAnimateSteps += Private::layoutAnimateStepsTotal;
		d->animateLayout = true;
	}
	slotLayoutAnimateItems();
}

void Item::slotLayoutAnimateItems()
{
	if ( ++d->layoutAnimateSteps == Private::layoutAnimateStepsTotal )
		d->layoutAnimateTimer.stop();

	const int s = Private::layoutAnimateStepsTotal;
	const int p = d->layoutAnimateSteps;

	updateAnimationPosition( p, s );
	setHeight(0);
	repaint();
}

float Item::opacity()
{
	return d->opacity;
}

void Item::setOpacity( float opacity )
{
	if ( d->opacity == opacity ) return;
	d->opacity = opacity;
	repaint();
}

bool Item::targetVisibility()
{
	return d->visibilityTarget;
}

void Item::setTargetVisibility( bool vis )
{
	if ( d->visibilityTarget == vis )
	{
		// in case we're getting called because our parent was shown and
		// we need to be rehidden
		if ( !d->visibilityTimer.isActive() )
			setVisible( vis );
		return;
	}
	d->visibilityTarget = vis;
	d->visibilityTimer.start( 40 );
	if ( targetVisibility() )
		setVisible( true );
	slotUpdateVisibility();
}

void Item::slotUpdateVisibility()
{
	if ( targetVisibility() )
		++d->visibilityLevel;
	else
		--d->visibilityLevel;

	if ( d->visibilityLevel >= Private::visibilityStepsTotal )
	{
		d->visibilityLevel = Private::visibilityStepsTotal;
		d->visibilityTimer.stop();
	}
	else if ( d->visibilityLevel <= 0 )
	{
		d->visibilityLevel = 0;
		d->visibilityTimer.stop();
		setVisible( false );
	}
	setHeight( 0 );
	repaint();
}

void Item::repaint()
{
	// if we're about to relayout, don't bother painting yet.
	if ( d->layoutTimer.isActive() )
		return;
	listView()->repaintItem( this );
}

void Item::relayout()
{
	scheduleLayout();
}

void Item::setup()
{
	KListViewItem::setup();
	slotLayoutItems();
}

void Item::setHeight( int )
{
	int minHeight = 0;
	for ( uint n = 0; n < components(); ++n )
		minHeight = QMAX( minHeight, component( n )->rect().height() );
	//kdDebug(14000) << k_funcinfo << "Height is " << minHeight << endl;
	if ( d->visibilityTimer.isActive() )
	{
		int vis = QMIN( d->visibilityLevel, Private::visibilityFoldSteps );
		minHeight = (minHeight * vis) / Private::visibilityFoldSteps;
	}
	KListViewItem::setHeight( minHeight );
}

void Item::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
	QPixmap back( width, height() );
	QPainter paint( &back );
	KListViewItem::paintCell( &paint, cg, column, width, align );
	if ( Component *comp = component( column ) )
		comp->paint( &paint, cg );
	paint.end();

#ifdef HAVE_XRENDER
	QColor rgb = cg.base();//backgroundColor();
	float opac = 1.0;
	if ( d->visibilityTimer.isActive() )
	{
		int vis = QMAX( d->visibilityLevel - Private::visibilityFoldSteps, 0 );
		opac = float(vis) / Private::visibilityFadeSteps;
	}
	opac *= opacity();
	const int alpha = 257 - int(opac * 257);
	if ( alpha != 0 )
	{
		XRenderColor clr = { alpha * rgb.red(), alpha * rgb.green(), alpha * rgb.blue(), alpha * 0xff };
		XRenderFillRectangle( back.x11Display(), PictOpOver, back.x11RenderHandle(),
		                      &clr, 0, 0, width, height() );
	}
#endif

	p->drawPixmap( 0, 0, back );
}

void Item::componentAdded( Component *component )
{
	ComponentBase::componentAdded( component );
	scheduleLayout();
}

void Item::componentRemoved( Component *component )
{
	ComponentBase::componentRemoved( component );
	scheduleLayout();
}

void Item::componentResized( Component *component )
{
	ComponentBase::componentResized( component );
	scheduleLayout();
}

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#include "kopetelistviewitem.moc"

// vim: set noet ts=4 sts=4 sw=4:
