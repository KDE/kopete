/*
    kopetetabwidget.h - KDE Tab Widget

    Copyright (c) 2003      by Jason Keirstead       <jason@keirstead.org>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETETABWIDGET_H
#define KOPETETABWIDGET_H

#include <qtabwidget.h>
#include <qtabbar.h>

class QPainter;
class QContextMenuEvent;
class KopeteTabBarPrivate;

class KopeteTabWidget : public QTabWidget
{
	Q_OBJECT

	public:
		KopeteTabWidget( QWidget * );

		/**
		* Sets the text color of a tab to the specified color.
		* Useful to indicate things such as widget state changes.
		* @param tab The tab widget to change the text color of
		* @param color The color to set the label
		*/
		void setLabelTextColor( QWidget *tab, const QColor &color ) const;

	signals:
		/**
		* Fired when a context menu event occurs on a tab.
		* Useful for a tab specfic popup menu.
		* @param target The widget within the tab the context menu was fired on
		* @param pos The position of the event
		*/
		void contextMenu( QWidget *target, const QPoint &pos );
};

class KopeteTabBar : public QTabBar
{
	Q_OBJECT

	public:
		KopeteTabBar( KopeteTabWidget * );
		~KopeteTabBar();

		/**
		* Sets the text color of a tab to the specified color.
		* Useful to indicate things such as widget state changes.
		* @param tabIndex The index of the tab to change the text color of
		* @param color The color to set the label
		*/
		void setLabelTextColor( int tabIndex,  const QColor &color  );

		virtual void removeTab( QTab* );

	signals:
		/**
		* Fired when a context menu event occurs on a tab.
		* Useful for a tab specfic popup menu.
		* @param target The widget within the tab the context menu was fired on
		* @param pos The position of the event
		*/
		void contextMenu( QWidget *target, const QPoint &pos );

	protected:
		virtual void paintLabel ( QPainter *, const QRect &, QTab *, bool ) const;
		void contextMenuEvent( QContextMenuEvent *e );

	private:
		KopeteTabBarPrivate *d;
};
#endif
