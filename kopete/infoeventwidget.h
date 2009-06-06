/*
    infoeventwidget.h - Info Event Widget

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef INFOEVENTWIDGET_H
#define INFOEVENTWIDGET_H

#include <QWidget>

namespace Kopete { class InfoEvent; }
/**
 * @author Roman Jarosz <kedgedev@centrum.cz>
 *
 * This dialog is used to view info events.
 */
class InfoEventWidget : public QWidget
{
Q_OBJECT
public:
	/**
	 * Info event widget constructor
	 */
	InfoEventWidget( QWidget *parent = 0 );

	~InfoEventWidget();

	virtual void setVisible( bool visible );

Q_SIGNALS:
	void showRequest();

public Q_SLOTS:
	/**
	 * Show previous info event from info event list
	 */
	void prevInfoEvent();

	/**
	 * Show next info event from info event list
	 */
	void nextInfoEvent();

	/**
	 * Close current info event from info event list
	 * @note Info Event object will be destroyed.
	 */
	void closeInfoEvent();

private Q_SLOTS:
	void slotAnimate( qreal amount );

	void linkClicked( const QString& link );

	/**
	 * Update widget's buttons and text.
	 */
	void updateInfo();

	void eventAdded( Kopete::InfoEvent* event );

	void notificationActivated();

	void notificationClosed();

private:
	int sizeHintHeight() const;

	class Private;
	Private * const d;
};

#endif
