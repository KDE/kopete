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

#include <limits>

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

bool Component::setMinWidth( int width )
{
	if ( d->minWidth == width ) return false;
	d->minWidth = width;
	
	d->parent->componentResized( this );
	return true;
}
bool Component::setMinHeight( int height )
{
	if ( d->minHeight == height ) return false;
	d->minHeight = height;

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
	{
		int width = 0;
		for ( uint n = 0; n < components(); ++n )
			width = QMAX( width, component( n )->widthForHeight( height ) );
		return width;
	}
	else
	{
		int width = (components() - 1) * Private::padding;
		for ( uint n = 0; n < components(); ++n )
			width += component( n )->widthForHeight( height );
		return width;
	}
}

int BoxComponent::heightForWidth( int width )
{
	if ( d->direction == Horizontal )
	{
		int height = 0;
		for ( uint n = 0; n < components(); ++n )
			height = QMAX( height, component( n )->heightForWidth( width ) );
		return height;
	}
	else
	{
		int height = (components() - 1) * Private::padding;
		for ( uint n = 0; n < components(); ++n )
			height += component( n )->heightForWidth( width );
		return height;
	}
}

void BoxComponent::calcMinSize()
{
	int sum = (components() - 1) * Private::padding, max = 0;
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
	}

	bool sizeChanged = false;
	if ( d->direction == Horizontal )
	{
		if ( setMinWidth( sum ) ) sizeChanged = true;
		if ( setMinHeight( max ) ) sizeChanged = true;
	}
	else
	{
		if ( setMinWidth( max ) ) sizeChanged = true;
		if ( setMinHeight( sum ) ) sizeChanged = true;
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
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		if ( horiz )
			fixedSize += comp->minWidth();
		else
			fixedSize += comp->minHeight();
	}

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
			int minWidth = comp->minWidth();
			int desiredWidth = comp->widthForHeight( rect.height() );
			rc.setWidth( QMIN( remaining + minWidth, desiredWidth ) );
			pos += rc.width();
			remaining -= rc.width() - minWidth;
		}
		else
		{
			rc.setLeft( rect.left() );
			rc.setTop( rect.top() + pos );
			rc.setWidth( rect.width() );
			int minHeight = comp->minHeight();
			int desiredHeight = comp->heightForWidth( rect.width() );
			rc.setHeight( QMIN( remaining + minHeight, desiredHeight ) );
			pos += rc.height();
			remaining -= rc.height() - minHeight;
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
	Private() : customColor( false ) {}
	QString text;
	bool customColor;
	QColor color;
	QFont font;
};

TextComponent::TextComponent( ComponentBase *parent, const QFont &font, const QString &text )
 : Component( parent ), d( new Private )
{
	setFont( font );
	setText( text );
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

void TextComponent::calcMinSize()
{
	setMinWidth( 0 );

	if ( !d->text.isEmpty() )
		setMinHeight( QFontMetrics( font() ).height() );
	else
		setMinHeight( 0 );

	repaint();
}

int TextComponent::widthForHeight( int )
{
	// add 2 to place an extra gap between the text and things to its right.
	// allegedly if this is not done the protocol icons overlap the text.
	// i however have never seen this problem (which would almost certainly
	// be a bug somewhere else).
	return QFontMetrics( font() ).width( d->text ) + 2;
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
	setMinWidth( 0 );
	setMinHeight( 0 );
}

int HSpacerComponent::widthForHeight( int )
{
	return std::numeric_limits<int>::max();
}

// VSpacerComponent --------

VSpacerComponent::VSpacerComponent( ComponentBase *parent )
 : Component( parent )
{
	setMinWidth( 0 );
	setMinHeight( 0 );
}

int VSpacerComponent::heightForWidth( int )
{
	return std::numeric_limits<int>::max();
}

// Item --------

/**
 * A periodic timer intended to be shared amongst multiple objects. Will run only
 * if an object is attached to it.
 */
class SharedTimer : private QTimer
{
	int period;
	int users;
public:
	SharedTimer( int period ) : period(period), users(0) {}
	void attach( QObject *target, const char *slot )
	{
		connect( this, SIGNAL(timeout()), target, slot );
		if( users++ == 0 )
			start( period );
		//kdDebug(14000) << "SharedTimer::attach: users is now " << users << "\n";
	}
	void detach( QObject *target, const char *slot )
	{
		disconnect( this, SIGNAL(timeout()), target, slot );
		if( --users == 0 )
			stop();
		//kdDebug(14000) << "SharedTimer::detach: users is now " << users << "\n";
	}
};

class SharedTimerRef
{
	SharedTimer &timer;
	QObject * const object;
	const char * const slot;
	bool attached;
public:
	SharedTimerRef( SharedTimer &timer, QObject *obj, const char *slot )
	 : timer(timer), object(obj), slot(slot), attached(false)
	{
	}
	void start()
	{
		if( attached ) return;
		timer.attach( object, slot );
		attached = true;
	}
	void stop()
	{
		if( !attached ) return;
		timer.detach( object, slot );
		attached = false;
	}
	bool isActive()
	{
		return attached;
	}
};

class Item::Private
{
public:
	Private( Item *item )
	 : layoutAnimateTimer( theLayoutAnimateTimer(), item, SLOT( slotLayoutAnimateItems() ) )
	 , animateLayout( true ), opacity( 1.0 )
	 , visibilityTimer( theVisibilityTimer(), item, SLOT( slotUpdateVisibility() ) )
	 , visibilityLevel( 0 ), visibilityTarget( false )
	{
	}

	QTimer layoutTimer;

	//QTimer layoutAnimateTimer;
	SharedTimerRef layoutAnimateTimer;
	SharedTimer &theLayoutAnimateTimer()
	{
		static SharedTimer timer( 10 );
		return timer;
	}

	bool animateLayout;
	int layoutAnimateSteps;
	static const int layoutAnimateStepsTotal = 10;

	float opacity;

	//QTimer visibilityTimer;
	SharedTimerRef visibilityTimer;
	SharedTimer &theVisibilityTimer()
	{
		static SharedTimer timer( 40 );
		return timer;
	}

	int visibilityLevel;
	bool visibilityTarget;
	static const int visibilityFoldSteps = 7;
#ifdef HAVE_XRENDER
	static const int visibilityFadeSteps = 7;
#else
	static const int visibilityFadeSteps = 0;
#endif
	static const int visibilityStepsTotal = visibilityFoldSteps + visibilityFadeSteps;
	static bool animateChanges;
	static bool fadeVisibility;
	static bool foldVisibility;
};

bool Item::Private::animateChanges = true;
bool Item::Private::fadeVisibility = true;
bool Item::Private::foldVisibility = true;

Item::Item( QListViewItem *parent, QObject *owner, const char *name )
 : QObject( owner, name ), KListViewItem( parent ), d( new Private(this) )
{
	initLVI();
}

Item::Item( QListView *parent, QObject *owner, const char *name )
 : QObject( owner, name ), KListViewItem( parent ), d( new Private(this) )
{
	initLVI();
}

Item::~Item()
{
	delete d;
}

void Item::setEffects( bool animation, bool fading, bool folding )
{
	Private::animateChanges = animation;
	Private::fadeVisibility = fading;
	Private::foldVisibility = folding;
}

void Item::initLVI()
{
	connect( listView()->header(), SIGNAL( sizeChange( int, int, int ) ), SLOT( slotColumnResized() ) );
	connect( &d->layoutTimer, SIGNAL( timeout() ), SLOT( slotLayoutItems() ) );
	//connect( &d->layoutAnimateTimer, SIGNAL( timeout() ), SLOT( slotLayoutAnimateItems() ) );
	//connect( &d->visibilityTimer, SIGNAL( timeout() ), SLOT( slotUpdateVisibility() ) );
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

	if ( Private::animateChanges && d->animateLayout && !d->visibilityTimer.isActive() )
	{
		d->layoutAnimateTimer.start();
		//if ( !d->layoutAnimateTimer.isActive() )
		//	d->layoutAnimateTimer.start( 10 );
		d->layoutAnimateSteps = 0;
	}
	else
	{
		d->layoutAnimateSteps = Private::layoutAnimateStepsTotal;
		d->animateLayout = true;
	}
	slotLayoutAnimateItems();
}

void Item::slotLayoutAnimateItems()
{
	if ( ++d->layoutAnimateSteps >= Private::layoutAnimateStepsTotal )
		d->layoutAnimateTimer.stop();

	const int s = Private::layoutAnimateStepsTotal;
	const int p = QMIN( d->layoutAnimateSteps, s );

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
	d->visibilityTimer.start();
	//d->visibilityTimer.start( 40 );
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

	if ( !Private::foldVisibility && !Private::fadeVisibility )
		d->visibilityLevel = targetVisibility() ? Private::visibilityStepsTotal : 0;
	else if ( !Private::fadeVisibility && d->visibilityLevel >= Private::visibilityFoldSteps )
		d->visibilityLevel = targetVisibility() ? Private::visibilityStepsTotal : Private::visibilityFoldSteps - 1;
	else if ( !Private::foldVisibility && d->visibilityLevel <= Private::visibilityFoldSteps )
		d->visibilityLevel = targetVisibility() ? Private::visibilityFoldSteps + 1 : 0;

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
	if ( Private::foldVisibility && d->visibilityTimer.isActive() )
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
	if ( d->visibilityTimer.isActive() && Private::fadeVisibility )
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
