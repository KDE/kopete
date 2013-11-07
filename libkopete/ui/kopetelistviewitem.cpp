/*
    kopetelistviewitem.cpp - Kopete's modular QListViewItems

    Copyright (c) 2005      by Engin AYDOGAN          <engin@bzzzt.biz>
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

#include "kopetelistviewitem.h"
#include "kopetecontact.h"
#include "kopeteemoticons.h"
#include "kopeteonlinestatus.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include <qapplication.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtimer.h>
#include <q3header.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <QList>

#include <limits.h>

namespace Kopete {
namespace UI {
namespace ListView {

// ComponentBase --------

class ComponentBase::Private
{
public:
	QList<Component*> components;
};

ComponentBase::ComponentBase()
 : d( new Private )
{
}

ComponentBase::~ComponentBase()
{
	foreach( Component *c , d->components )
		delete c;
	delete d;
}

uint ComponentBase::components() { return d->components.count(); }

Component *ComponentBase::component( uint n )
{
	if( n < components() )
		return d->components.at( n );
	return 0l;

}

Component *ComponentBase::componentAt( const QPoint &pt )
{
	foreach( Component* n , d->components )
	{
		if ( n->rect().contains( pt ) )
		{
			if ( Component *comp = n->componentAt( pt ) )
				return comp;
			return n;
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
	d->components.removeAll( component );
}

void ComponentBase::clear()
{
	foreach( Component *c , d->components )
		delete c;
	d->components.clear();
}

void ComponentBase::componentResized( Component * )
{
}

std::pair<QString,QRect> ComponentBase::toolTip( const QPoint &relativePos )
{
	foreach( Component* n , d->components )
		if ( n->rect().contains( relativePos ) )
			return n->toolTip( relativePos );

	return std::make_pair( QString(), QRect() );
}

void ComponentBase::updateAnimationPosition( int p, int s )
{
	foreach( Component* comp , d->components )
	{
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
	 , tipSource( 0 )
	{
	}
	ComponentBase *parent;
	QRect rect;
	QRect startRect, targetRect;
	int minWidth, minHeight;
	bool growHoriz, growVert;
	bool show;
	ToolTipSource *tipSource;
};

Component::Component( ComponentBase *parent )
 : d( new Private( parent ) )
{
	d->parent->componentAdded( this );
	d->show = true;
}

int Component::RTTI = Rtti_Component;

Component::~Component()
{
	d->parent->componentRemoved( this );
	delete d;
}


void Component::hide()
{
	d->show = false;
}

void Component::show()
{
	d->show = true;
}

bool Component::isShown()
{
	return d->show;
}

bool Component::isHidden()
{
	return !d->show;
}

void Component::setToolTipSource( ToolTipSource *source )
{
	d->tipSource = source;
}

std::pair<QString,QRect> Component::toolTip( const QPoint &relativePos )
{
	if ( !d->tipSource )
		return ComponentBase::toolTip( relativePos );

	QRect rc = rect();
	QString result = (*d->tipSource)( this, relativePos, rc );
	return std::make_pair(result, rc);
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
	//kDebug(14000) << "At " << rect;
}

void Component::setRect( const QRect &rect )
{
	d->rect = rect;
}

void Component::paint( QPainter *painter, const QPalette &pal )
{
	/*painter->setPen( Qt::red );
	painter->drawRect( rect() );*/
	for ( uint n = 0; n < components(); ++n )
	{
		if( component( n )->isShown() )
			component( n )->paint( painter, pal );
	}
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

int BoxComponent::RTTI = Rtti_BoxComponent;

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
			width = qMax( width, component( n )->widthForHeight( height ) );
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
			height = qMax( height, component( n )->heightForWidth( width ) );
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
			max = qMax( max, comp->minHeight() );
			sum += comp->minWidth();
		}
		else
		{
			max = qMax( max, comp->minWidth() );
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
		total = qMax( rect.width(), minWidth() );
	else
		total = qMax( rect.height(), minHeight() );

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
			rc.setWidth( qMin( remaining + minWidth, desiredWidth ) );
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
			rc.setHeight( qMin( remaining + minHeight, desiredHeight ) );
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

int ImageComponent::RTTI = Rtti_ImageComponent;

ImageComponent::ImageComponent( ComponentBase *parent, int minW, int minH )
 : Component( parent ), d( new Private )
{
	setMinWidth( minW );
	setMinHeight( minH );
	repaint();
}

ImageComponent::~ImageComponent()
{
	delete d;
}

QPixmap ImageComponent::pixmap()
{
	return d->image;
}

void ImageComponent::setPixmap( const QPixmap &img, bool adjustSize)
{
	d->image = img;
	if ( adjustSize )
	{
		setMinWidth( img.width() );
		setMinHeight( img.height() );
	}
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

void ImageComponent::paint( QPainter *painter, const QPalette &pal )
{
	Q_UNUSED( pal )
	QRect ourRc = rect();
	QRect rc = d->image.rect();
	// center rc within our rect
	rc.moveTopLeft( ourRc.topLeft() + (ourRc.size() - rc.size()) / 2 );
	// paint, shrunk to be within our rect
	painter->drawPixmap( rc & ourRc, d->image );
}

void ImageComponent::scale( int w, int h, Qt::AspectRatioMode mode )
{
	QImage im = d->image.toImage();
	setPixmap( QPixmap::fromImage( im.scaled( w, h, mode, Qt::SmoothTransformation) ) );
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

int TextComponent::RTTI = Rtti_TextComponent;

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
	relayout();
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
	return d->customColor ? d->color : QColor();
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

void TextComponent::paint( QPainter *painter, const QPalette &pal )
{
	if ( d->customColor )
		painter->setPen( d->color );
	else
		painter->setPen( pal.text().color() );

	QFontMetrics metrics( font() );
	QString dispStr = metrics.elidedText( d->text, Qt::ElideRight, rect().width() );
	painter->setFont( font() );
	painter->drawText( rect(), Qt::TextSingleLine, dispStr );
}

// DisplayNameComponent

class DisplayNameComponent::Private
{
public:
	QString text;
    QFont font;
};

DisplayNameComponent::DisplayNameComponent( ComponentBase *parent )
 : BoxComponent( parent ), d( new Private )
{
}

int DisplayNameComponent::RTTI = Rtti_DisplayNameComponent;

DisplayNameComponent::~DisplayNameComponent()
{
	delete d;
}

void DisplayNameComponent::layout( const QRect &rect )
{
	Component::layout( rect );

	// finally, lay everything out
	QRect rc;
	int totalWidth = rect.width();
	int usedWidth = 0;
	bool exceeded = false;
	for ( uint n = 0; n < components(); ++n )
	{
		Component *comp = component( n );
		if ( !exceeded )
		{
			if ( ( usedWidth + comp->widthForHeight( rect.height() ) ) > totalWidth )
			{
				exceeded = true;
				// TextComponents can squeeze themselves
				if ( comp->rtti() == Rtti_TextComponent )
				{
					comp->show();
					comp->layout( QRect( usedWidth+ rect.left(), rect.top(),
							     totalWidth - usedWidth,
							     comp->heightForWidth( totalWidth - usedWidth ) ) );
				} else {
					comp->hide();
				}
			}
			else
			{
				comp->show();
				comp->layout( QRect( usedWidth+ rect.left(), rect.top(),
						     comp->widthForHeight( rect.height() ),
						     comp->heightForWidth( rect.width() ) ) );
			}
			usedWidth+= comp->widthForHeight( rect.height() );
		}
		else
		{
			// Shall we implement a hide()/show() in Component class ?
			comp->hide();
		}
	}
}

void DisplayNameComponent::setText( const QString& text )
{
	if ( d->text == text )
		return;
	d->text = text;

	redraw();
}

void DisplayNameComponent::redraw()
{
	QColor color;
	for ( uint n = 0; n < components(); ++n )
	{
		if( component( n )->rtti() == Rtti_TextComponent )
			color = ((TextComponent*)component(n))->color();
	}

	QList<KEmoticonsTheme::Token> tokens;
	QList<KEmoticonsTheme::Token>::const_iterator token;

	clear(); // clear childen

	tokens = Kopete::Emoticons::tokenize( d->text );
	ImageComponent *ic;

	QFontMetrics fontMetrics( d->font );
	int fontHeight = fontMetrics.height();
	for ( token = tokens.constBegin(); token != tokens.constEnd(); ++token )
	{
		switch ( (*token).type )
		{
		case KEmoticonsTheme::Text:
			new TextComponent( this,  d->font, (*token).text );
		break;
		case KEmoticonsTheme::Image:
			ic = new ImageComponent( this );
			ic->setPixmap( QPixmap( (*token).picPath ) );
			ic->scale( INT_MAX, fontHeight, Qt::KeepAspectRatio );
		break;
		default:
			kDebug( 14010 ) << "This should have not happened!";
		}
	}

	if(color.isValid())
		setColor( color );
}

void DisplayNameComponent::setFont( const QFont& font )
{
	for ( uint n = 0; n < components(); ++n )
		if( component( n )->rtti() == Rtti_TextComponent )
			((TextComponent*)component(n))->setFont( font );
	d->font = font;
}

void DisplayNameComponent::setColor( const QColor& color )
{
	for ( uint n = 0; n < components(); ++n )
		if( component( n )->rtti() == Rtti_TextComponent )
			((TextComponent*)component(n))->setColor( color );
}

void DisplayNameComponent::setDefaultColor()
{
	for ( uint n = 0; n < components(); ++n )
		if( component( n )->rtti() == Rtti_TextComponent )
			((TextComponent*)component(n))->setDefaultColor();
}

QString DisplayNameComponent::text()
{
	return d->text;
}

// HSpacerComponent --------

HSpacerComponent::HSpacerComponent( ComponentBase *parent )
 : Component( parent )
{
	setMinWidth( 0 );
	setMinHeight( 0 );
}

int HSpacerComponent::RTTI = Rtti_HSpacerComponent;

int HSpacerComponent::widthForHeight( int )
{
	return INT_MAX;
}

// VSpacerComponent --------

VSpacerComponent::VSpacerComponent( ComponentBase *parent )
 : Component( parent )
{
	setMinWidth( 0 );
	setMinHeight( 0 );
}

int VSpacerComponent::RTTI = Rtti_VSpacerComponent;

int VSpacerComponent::heightForWidth( int )
{
	return INT_MAX;
}

//////////////////  ContactComponent /////////////////////////

class ContactComponent::Private
{
public:
	Kopete::Contact *contact;
	int iconSize;
};

ContactComponent::ContactComponent( ComponentBase *parent, Kopete::Contact *contact, int iconSize) : ImageComponent( parent ) , d( new Private )
{
	d->contact = contact;
	d->iconSize = iconSize;
	updatePixmap();
}

ContactComponent::~ContactComponent()
{
	delete d;
}

void ContactComponent::updatePixmap()
{
	setPixmap( contact()->onlineStatus().iconFor( contact() ).pixmap( d->iconSize ) );
}
Kopete::Contact *ContactComponent::contact()
{
	return d->contact;
}

// we don't need to use a tooltip source here - this way is simpler
std::pair<QString,QRect> ContactComponent::toolTip( const QPoint &/*relativePos*/ )
{
	return std::make_pair(d->contact->toolTip(),rect());
}

//////////////////  SpacerComponent /////////////////////////

SpacerComponent::SpacerComponent( ComponentBase *parent, int w, int h ) : Component( parent )
{
	setMinWidth(w);
	setMinHeight(h);
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
		//kDebug(14000) << "SharedTimer::attach: users is now " << users << "\n";
	}
	void detach( QObject *target, const char *slot )
	{
		disconnect( this, SIGNAL(timeout()), target, slot );
		if( --users == 0 )
			stop();
		//kDebug(14000) << "SharedTimer::detach: users is now " << users << "\n";
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
	 : layoutAnimateTimer( theLayoutAnimateTimer(), item, SLOT(slotLayoutAnimateItems()) )
	 , animateLayout( true ), opacity( 1.0 )
	 , visibilityTimer( theVisibilityTimer(), item, SLOT(slotUpdateVisibility()) )
	 , visibilityLevel( 0 ), visibilityTarget( false ), searchMatch( true )
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
	static const int visibilityFadeSteps = 7;
	static const int visibilityStepsTotal = visibilityFoldSteps + visibilityFadeSteps;

	bool searchMatch;

	static bool animateChanges;
	static bool fadeVisibility;
	static bool foldVisibility;
};

bool Item::Private::animateChanges = true;
bool Item::Private::fadeVisibility = true;
bool Item::Private::foldVisibility = true;

Item::Item( Q3ListViewItem *parent, QObject *owner )
 : QObject( owner ), K3ListViewItem( parent ), d( new Private(this) )
{
	initLVI(parent->listView());
}

Item::Item( Q3ListView *parent, QObject *owner )
 : QObject( owner ), K3ListViewItem( parent ), d( new Private(this) )
{
	initLVI(parent);
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

void Item::initLVI(QObject* parent)
{
	connect( listView()->header(), SIGNAL(sizeChange(int,int,int)), SLOT(slotColumnResized()) );
	connect( &d->layoutTimer, SIGNAL(timeout()), SLOT(slotLayoutItems()) );
	connect (this, SIGNAL (visibilityChanged(bool)), parent, SIGNAL (visibleSizeChanged()) );
	//connect( &d->layoutAnimateTimer, SIGNAL(timeout()), SLOT(slotLayoutAnimateItems()) );
	//connect( &d->visibilityTimer, SIGNAL(timeout()), SLOT(slotUpdateVisibility()) );
	mySetVisible( false );
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
	{
		d->layoutTimer.setSingleShot( true );
		d->layoutTimer.start( 30 );
	}
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
		//kDebug(14000) << "Component " << n << " is " << width << " x " << height;
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
	const int p = qMin( d->layoutAnimateSteps, s );

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

void Item::setSearchMatch( bool match, bool searching )
{
	d->searchMatch = match;

	if ( !match )
		mySetVisible( false );
	else
	{
		kDebug(14000) << " match: " << match << ", vis timer active: " << d->visibilityTimer.isActive()
		               << ", target visibility: " << targetVisibility() << endl;
		if ( d->visibilityTimer.isActive() || searching )
			mySetVisible( true );
		else
			mySetVisible( targetVisibility() );
	}
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
			mySetVisible( vis && d->searchMatch );
		return;
	}
	d->visibilityTarget = vis;
	d->visibilityTimer.start();
	//d->visibilityTimer.start( 40 );
	if ( targetVisibility() )
		mySetVisible( d->searchMatch );
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
		mySetVisible( false );
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
	K3ListViewItem::setup();
	slotLayoutItems();
}

void Item::setHeight( int )
{
	int minHeight = 0;
	for ( uint n = 0; n < components(); ++n )
		minHeight = qMax( minHeight, component( n )->rect().height() );
	//kDebug(14000) << "Height is " << minHeight;
	if ( Private::foldVisibility && d->visibilityTimer.isActive() )
	{
		int vis = d->visibilityLevel;
		if ( vis > Private::visibilityFoldSteps )
		    vis = Private::visibilityFoldSteps;
		minHeight = (minHeight * vis) / Private::visibilityFoldSteps;
	}
	K3ListViewItem::setHeight( minHeight );
}

int Item::width( const QFontMetrics &, const Q3ListView *lv, int c ) const
{
	// Qt computes the itemRect from this. we want the whole item to be
	// clickable, so we return the widest we could possibly be.
	return lv->header()->sectionSize( c );
}

void Item::paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align )
{
	Q_UNUSED(align);
	QPixmap back( width, height() );
	QPainter paint( &back );
	//K3ListViewItem::paintCell( &paint, cg, column, width, align );
	// PASTED FROM KLISTVIEWITEM:
	// set the alternate cell background colour if necessary
	QColorGroup _cg = cg;
	_cg.setColor( listView()->backgroundRole(), backgroundColor(column) );

// PASTED FROM QLISTVIEWITEM
	{
		QPainter *p = &paint;

		Q3ListView *lv = listView();
		if ( !lv )
			return;
		QFontMetrics fm( p->fontMetrics() );

		// any text we render is done by the Components, not by this class, so make sure we've nothing to write
		QString t;

		// removed text truncating code from Qt - we do that differently, further on

		int marg = lv->itemMargin();
		QBrush b;
		if (isSelected())
			b = _cg.brush(QPalette::Normal, QPalette::Highlight);
		else
			b = _cg.background();
		p->fillRect( 0, 0, width, height(), b );
	//	const QPixmap * icon = pixmap( column );
#ifdef __GNUC__
#warning Item::paintCell needs fixing
#endif
/*
        const QPalette::ColorRole crole = backgroundRole();

		if ( _cg.brush( crole ) != lv->colorGroup().brush( crole ) )
			p->fillRect( 0, 0, width, height(), _cg.brush( crole ) );
		else
		{
			// all copied from QListView::paintEmptyArea

			//lv->paintEmptyArea( p, QRect( 0, 0, width, height() ) );
			QStyleOption opt( lv->sortColumn(), 0 ); // ### hack; in 3.1, add a property in QListView and QHeader
			QStyle::SFlags how = QStyle::State_Default;
			if ( lv->isEnabled() )
				how |= QStyle::State_Enabled;

			lv->style()->drawComplexControl( QStyle::CC_Q3ListView,
						p, lv, QRect( 0, 0, width, height() ), lv->colorGroup(),
						how, QStyle::SC_Q3ListView, QStyle::SC_None,
						opt );
		}



		if ( isSelected() &&
		(column == 0 || lv->allColumnsShowFocus()) ) {
			p->fillRect( r - marg, 0, width - r + marg, height(),
					_cg.brush( QPalette::Highlight ) );
	// removed text pen setting code from Qt
		}

		// removed icon drawing code from Qt

		// draw the tree gubbins
		if ( multiLinesEnabled() && column == 0 && isOpen() && childCount() ) {
			int textheight = fm.size( align, t ).height() + 2 * lv->itemMargin();
			textheight = qMax( textheight, QApplication::globalStrut().height() );
			if ( textheight % 2 > 0 )
				textheight++;
			if ( textheight < height() ) {
				int w = lv->treeStepSize() / 2;
				lv->style()->drawComplexControl( QStyle::CC_Q3ListView, p, lv,
								QRect( 0, textheight, w + 1, height() - textheight + 1 ), _cg,
								lv->isEnabled() ? QStyle::State_Enabled : QStyle::State_Default,
								QStyle::SC_Q3ListViewExpand,
								(uint)QStyle::SC_All, QStyleOption( this ) );
			}
        }
       */
	}
	// END OF PASTE


	//do you see a better way to tell the TextComponent we are selected ?  - Olivier 2004-09-02
	if ( isSelected() )
		_cg.setColor(QPalette::Text , _cg.highlightedText() );

	if ( Component *comp = component( column ) )
		comp->paint( &paint, _cg );
	paint.end();

	QColor rgba = cg.base();//backgroundColor();
	float opac = 1.0;
	if ( d->visibilityTimer.isActive() && Private::fadeVisibility )
	{
		int vis = d->visibilityLevel - Private::visibilityFoldSteps;
		if ( vis < 0 )
		    vis = 0;

		opac = float(vis) / Private::visibilityFadeSteps;
	}
	opac *= opacity();
	const int alpha = 255 - int(opac * 255);
	if ( alpha != 0 )
	{
		rgba.setAlpha(alpha);
		QPainter p(&back);
		p.fillRect(back.rect(), rgba);
	}

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

void Item::mySetVisible ( bool b )
{
	setVisible (b);
	emit visibilityChanged (b);
}

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#include "kopetelistviewitem.moc"

// vim: set noet ts=4 sts=4 sw=4:
