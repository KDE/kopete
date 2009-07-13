/*
    kopetelistviewitem.h - Kopete's modular QListViewItems

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

#ifndef KOPETE_LISTVIEWITEM_H
#define KOPETE_LISTVIEWITEM_H

#include <k3listview.h>
#include <kopetecontact.h>
#include <kopete_export.h>

#include <utility>

#include <QtGui/QPixmap>

namespace Kopete {
namespace UI {
namespace ListView {

class Component;

class KOPETE_EXPORT ComponentBase
{
public:
	ComponentBase();
	virtual ~ComponentBase() = 0;

	uint components();
	Component *component( uint n );
	Component *componentAt( const QPoint &pt );

	/** Repaint this item */
	virtual void repaint() = 0;
	/** Relayout this item */
	virtual void relayout() = 0;

	/**
	 * Get the tool tip string and rectangle for a tip request at position
	 * relativePos relative to this item. Queries the appropriate child component.
	 *
	 * @return a pair where the first element is the tooltip, and the second is
	 *         the rectangle within the item for which the tip should be displayed.
	 */
	virtual std::pair<QString,QRect> toolTip( const QPoint &relativePos );

protected:
	/** A child item has been added to this item */
	virtual void componentAdded( Component *component );
	/** A child item has been removed from this item */
	virtual void componentRemoved( Component *component );
	/** A child item has been resized */
	virtual void componentResized( Component *component );
	/** Remove all children */
	virtual void clear();

	/** @internal animate items */
	void updateAnimationPosition( int p, int s );
private:
	class Private;
	Private * const d;

	// calls componentAdded and componentRemoved
	friend class Component;
};

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KOPETE_EXPORT ToolTipSource
{
public:
	virtual ~ToolTipSource(){}
	/**
	 * Get the tooltip string and rect for a component
	 *
	 * @param component The component to get a tip for
	 * @param pt   The point (relative to the list item) the mouse is at
	 * @param rect The tip will be removed when the mouse leaves this rect.
	 *             Will initially be set to \p component's rect().
	 */
	virtual QString operator() ( ComponentBase *component, const QPoint &pt, QRect &rect ) = 0;
};

/**
 * This class represents a rectangular subsection of a ListItem.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KOPETE_EXPORT Component : public ComponentBase
{
protected:
	Component( ComponentBase *parent );
public:
	virtual ~Component() = 0;

	/**
	 * Set the size and position of this item relative to the list view item. Should
	 * only be called by the containing item.
	 * @param rect the new rectangle this component will paint in, relative to the painter
	 *             passed to the paint() function by the parent item.
	 */
	virtual void layout( const QRect &rect );

	/**
	 * Paint this item, inside the rectangle returned by rect().
	 * The default implementation calls paint on all children.
	 */
	virtual void paint( QPainter *painter, const QPalette &pal );

	void repaint();
	void relayout();

	/**
	 * @return the rect this component was allocated last time it was laid out
	 */
	QRect rect();
	/**
	 * Prevents this component to be drawn
	 */
	void hide();

	/**
	 * Makes this component to be drawn
	 */
	void show();

	bool isShown();
	bool isHidden();

	/**
	 * Returns the smallest this component can become horizontally while still
	 * being useful.
	 */
	int minWidth();
	/**
	 * Returns the smallest this component can become vertically while still
	 * being useful.
	 */
	int minHeight();

	/**
	 * Returns the width this component desires for a given @a height. By default
	 * this function returns minWidth().
	 */
	virtual int widthForHeight( int height );
	/**
	 * Returns the height this component desires for a given @a width. By default
	 * this function returns minHeight().
	 */
	virtual int heightForWidth( int width );

	/**
	 * Set a tool tip source for this item. The tool tip source object is
	 * still owned by the caller, and must live for at least as long as
	 * this component.
	 */
	void setToolTipSource( ToolTipSource *source = 0 );

	/**
	 * Get the tool tip string and rectangle for a tip request at position
	 * relativePos relative to this item. If a tooltip source is set, it will
	 * be used. Otherwise calls the base class.
	 *
	 * @return a pair where the first element is the tooltip, and the second is
	 *         the rectangle within the item for which the tip should be displayed.
	 */
	std::pair<QString,QRect> toolTip( const QPoint &relativePos );

	/**
	 * RTTI: Runtime Type Information
	 * Exactly the same as Qt's approach to identify types of
	 * QCanvasItems.
	 */

	enum RttiValues {
		Rtti_Component, Rtti_BoxComponent, Rtti_TextComponent,
		Rtti_ImageComponent, Rtti_DisplayNameComponent,
		Rtti_HSpacerComponent, Rtti_VSpacerComponent
	};

	static int RTTI;
	virtual int rtti() const { return RTTI; }
protected:
	/**
	 * Change the minimum width, in pixels, this component requires in order
	 * to be at all useful. Note: do not call this from your layout() function.
	 * @param width the minimum width
	 * @return true if the size has actually changed, false if it's been set to
	 *         the existing values. if it returns true, you do not need to relayout,
	 *         since the parent component will do that for you.
	 */
	bool setMinWidth( int width );
	/**
	 * Change the minimum height, in pixels, this component requires in order
	 * to be at all useful. Note: do not call this from your layout() function.
	 * @param height the minimum height
	 * @return true if the size has actually changed, false if it's been set to
	 *         the existing values. If it returns true, you do not need to relayout,
	 *         since the parent component will do that for you.
	 */
	bool setMinHeight( int height );

	void componentAdded( Component *component );
	void componentRemoved( Component *component );

private:
	// calls the three functions below
	friend void ComponentBase::updateAnimationPosition( int p, int s );

	// used for animation
	void setRect( const QRect &rect );
	QRect startRect();
	QRect targetRect();

	class Private;
	Private * const d;
};

class KOPETE_EXPORT BoxComponent : public Component
{
public:
	enum Direction { Horizontal, Vertical };
	explicit BoxComponent( ComponentBase *parent, Direction dir = Horizontal );
	~BoxComponent();

	void layout( const QRect &rect );

	virtual int widthForHeight( int height );
	virtual int heightForWidth( int width );

	static int RTTI;
	virtual int rtti() const { return RTTI; }

protected:
	void componentAdded( Component *component );
	void componentRemoved( Component *component );
	void componentResized( Component *component );

private:
	void calcMinSize();

	class Private;
	Private * const d;
};

class KOPETE_EXPORT TextComponent : public Component
{
public:
	explicit TextComponent( ComponentBase *parent, const QFont &font = QFont(), const QString &text = QString() );
	~TextComponent();

	QString text();
	void setText( const QString &text );

	QFont font();
	void setFont( const QFont &font );

	QColor color();
	void setColor( const QColor &color );
	void setDefaultColor();

	int widthForHeight( int );

	void paint( QPainter *painter, const QPalette &pal );

	static int RTTI;
	virtual int rtti() const { return RTTI; }

private:
	void calcMinSize();

	class Private;
	Private * const d;
};

class KOPETE_EXPORT ImageComponent : public Component
{
public:
	explicit ImageComponent( ComponentBase *parent );
	ImageComponent( ComponentBase *parent, int minW, int minH );
	~ImageComponent();

	void setPixmap( const QPixmap &img, bool adjustSize = true);
	QPixmap pixmap( void );

	void paint( QPainter *painter, const QPalette &pal );

	void scale( int w, int h, Qt::AspectRatioMode );
	static int RTTI;
	virtual int rtti() const { return RTTI; }
private:
	class Private;
	Private * const d;
};

/**
 * ContactComponent
 */
class KOPETE_EXPORT ContactComponent : public ImageComponent
{
public:
	ContactComponent( ComponentBase *parent, Kopete::Contact *contact, int iconSize);
	~ContactComponent();
	void updatePixmap();
	Kopete::Contact *contact();
	std::pair<QString,QRect> toolTip( const QPoint &relativePos );
protected:
	class Private;
	Private * const d;
};

/**
 * SpacerComponent
 */
class KOPETE_EXPORT SpacerComponent : public Component
{
public:
	SpacerComponent( ComponentBase *parent, int w, int h );
};

/**
 * DisplayNameComponent
 */

class KOPETE_EXPORT DisplayNameComponent : public BoxComponent
{
public:
	/**
	 * Constructor
	 */
	explicit DisplayNameComponent( ComponentBase *parent );

	/**
	 * Dtor
	 */
	~DisplayNameComponent();
	void layout( const QRect& rect );

	QString text();
	void setText( const QString& text );
	void setFont( const QFont& font );
	void setColor( const QColor& color );
	void setDefaultColor();
	static int RTTI;
	virtual int rtti() const { return RTTI; }
	/**
	 * reparse again for emoticon (call this when emoticon theme change)
	 */
	void redraw();

private:
	class Private;
	Private * const d;
};

class KOPETE_EXPORT HSpacerComponent : public Component
{
public:
	explicit HSpacerComponent( ComponentBase *parent );
	int widthForHeight( int );

	static int RTTI;
	virtual int rtti() const { return RTTI; }
};

class KOPETE_EXPORT VSpacerComponent : public Component
{
public:
	explicit VSpacerComponent( ComponentBase *parent );
	int heightForWidth( int );

	static int RTTI;
	virtual int rtti() const { return RTTI; }
};

/**
 * List-view item composed of Component items. Supports height-for-width, tooltips and
 * some animation effects.
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class KOPETE_EXPORT Item : public QObject, public K3ListViewItem, public ComponentBase
{
	Q_OBJECT
public:
	explicit Item( Q3ListView *parent, QObject *owner = 0 );
	explicit Item( Q3ListViewItem *parent, QObject *owner = 0 );
	~Item();

	void repaint();
	void relayout();

	void setup();
	virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
	//TODO: startRename(...)

	float opacity();
	void setOpacity( float alpha );

	bool targetVisibility();
	void setTargetVisibility( bool vis );

	/**
	 * Turn on and off certain visual effects for all Items.
	 * @param animation whether changes to items should be animated.
	 * @param fading whether requests to setTargetVisibility should cause fading of items.
	 * @param folding whether requests to setTargetVisibility should cause folding of items.
	 */
	static void setEffects( bool animation, bool fading, bool folding );

	int width( const QFontMetrics & fm, const Q3ListView * lv, int c ) const;

	/**
	 * Show or hide this item in a clean way depending on whether it matches
	 * the current quick search
	 * @param match If true, show or hide the item as normal. If false, hide the item immediately.
	 */
	virtual void setSearchMatch( bool match, bool searching );

protected:
	void componentAdded( Component *component );
	void componentRemoved( Component *component );
	void componentResized( Component *component );

	void setHeight( int );
	
signals:
	void visibilityChanged (bool visibility);

private:
	void initLVI(QObject* parent);
	void recalcHeight();
	void scheduleLayout();
	void mySetVisible ( bool b );

private slots:
	void slotColumnResized();
	void slotLayoutItems();
	void slotLayoutAnimateItems();
	void slotUpdateVisibility();

private:
	class Private;
	Private * const d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
