/*
    kopetelistviewitem.h - Kopete's modular QListViewItems

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

#include <klistview.h>

class QPixmap;

namespace Kopete {
namespace UI {
namespace ListView {

class Component;

class ComponentBase
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
protected:
	/** A child item has been added to this item */
	virtual void componentAdded( Component *component );
	/** A child item has been removed from this item */
	virtual void componentRemoved( Component *component );
	/** A child item has been resized */
	virtual void componentResized( Component *component );

	/** @internal animate items */
	void updateAnimationPosition( int p, int s );
private:
	class Private;
	Private *d;

	// calls componentAdded and componentRemoved
	friend class Component;
};

/**
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class Component : public ComponentBase
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
	virtual void paint( QPainter *painter, const QColorGroup &cg );

	void repaint();
	void relayout();

	/**
	 * @return the rect this component was allocated last time it was laid out
	 */
	QRect rect();

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
	 * Returns true if this component can make use of more horizontal space
	 */
	bool stretchHoriz();
	/**
	 * Returns true if this component can make use of more vertical space
	 */
	bool stretchVert();

	/**
	 * Request the width this component desires for a given @a height. By default
	 * this function returns minWidth().
	 */
	virtual int widthForHeight( int height );
	/**
	 * Request the height this component desires for a given @a width. By default
	 * this function returns minHeight().
	 */
	virtual int heightForWidth( int width );
	
protected:
	/**
	 * Change the minimum width, in pixels, this component requires in order
	 * to be at all useful. Note: do not call this from your layout() function.
	 * @param width the minimum width
	 * @param canUseMore set to true if this component can usefully use more
	 *        pixels in the X direction, false otherwise.
	 * @return true if the size has actually changed, false if it's been set to
	 *         the previous values. if it returns true, you do not need to relayout,
	 *         since the parent component will do that for you.
	 */
	bool setMinWidth( int width, bool canUseMore = false );
	/**
	 * Change the minimum height, in pixels, this component requires in order
	 * to be at all useful. Note: do not call this from your layout() function.
	 * @param height the minimum height
	 * @param canUseMore set to true if this component can usefully use more
	 *        pixels in the Y direction, false otherwise.
	 * @return true if the size has actually changed, false if it's been set to
	 *         the previous values. If it returns true, you do not need to relayout,
	 *         since the parent component will do that for you.
	 */
	bool setMinHeight( int height, bool canUseMore = false );

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
	Private *d;
};

class BoxComponent : public Component
{
public:
	enum Direction { Horizontal, Vertical };
	BoxComponent( ComponentBase *parent, Direction dir = Horizontal );
	~BoxComponent();

	void layout( const QRect &rect );
	
	int widthForHeight( int height );
	int heightForWidth( int width );

protected:
	void componentAdded( Component *component );
	void componentRemoved( Component *component );
	void componentResized( Component *component );

private:
	void calcMinSize();

	class Private;
	Private *d;
};

class TextComponent : public Component
{
public:
	TextComponent( ComponentBase *parent, const QFont &font, const QString &text = QString::null, bool fixedWidth = false );
	~TextComponent();

	QString text();
	void setText( const QString &text );

	QFont font();
	void setFont( const QFont &font );

	QColor color();
	void setColor( const QColor &color );
	void setDefaultColor();

	bool fixedWidth();
	void setFixedWidth( bool fixedWidth );

	void paint( QPainter *painter, const QColorGroup &cg );

private:
	void calcMinSize();

	class Private;
	Private *d;
};

class ImageComponent : public Component
{
public:
	ImageComponent( ComponentBase *parent );
	~ImageComponent();

	void setPixmap( const QPixmap &img );
	void paint( QPainter *painter, const QColorGroup &cg );

private:
	class Private;
	Private *d;
};

class HSpacerComponent : public Component
{
public:
	HSpacerComponent( ComponentBase *parent );
};

class VSpacerComponent : public Component
{
public:
	VSpacerComponent( ComponentBase *parent );
};

/**
 * List-view item composed of Component items. Supports height-for-width,
 * neat animation effects, drag-and-drop, in-place editing of text components, ...
 *
 * @author Richard Smith <kde@metafoo.co.uk>
 */
class Item : public QObject, public KListViewItem, public ComponentBase
{
	Q_OBJECT
public:
	Item( QListView *parent, QObject *owner = 0, const char *name = 0 );
	Item( QListViewItem *parent, QObject *owner = 0, const char *name = 0  );
	~Item();

	void repaint();
	void relayout();

	void setup();
	void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align );
	//TODO: startRename(...)

	float opacity();
	void setOpacity( float alpha );

	bool targetVisibility();
	void setTargetVisibility( bool vis );

protected:
	void componentAdded( Component *component );
	void componentRemoved( Component *component );
	void componentResized( Component *component );

	void setHeight( int );

private:
	void initLVI();
	void recalcHeight();

private slots:
	void slotScheduleLayout();
	void slotLayoutItems();
	void slotLayoutAnimateItems();
	void slotUpdateVisibility();

private:
	class Private;
	Private *d;
};

} // END namespace ListView
} // END namespace UI
} // END namespace Kopete

#endif

// vim: set noet ts=4 sts=4 sw=4:
