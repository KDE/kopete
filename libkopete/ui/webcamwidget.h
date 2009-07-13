/*
    webcamwidget.h - A simple widget for displaying webcam frames

    Copyright (c) 2006 by Gustavo Pichorim Boiko   <gustavo.boiko@kdemail.net>
    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef WEBCAMWIDGET_H 
#define WEBCAMWIDGET_H 

#include <qwidget.h>
#include <qpixmap.h>
#include <qstring.h>

#include "kopete_export.h"

namespace Kopete
{
/**
 * A simple widget to display webcam frames.
 */
class KOPETE_EXPORT WebcamWidget : public QWidget
{
	Q_OBJECT
public:
	/**
	* @brief WebcamWidget constructor.
	* @param parent The parent widget of this widget
	*/
	WebcamWidget(QWidget* parent = 0);
	~WebcamWidget();

	/**
	 * @brief Updates the frame being displayed in the widget
	 * @param pixmap The frame to be displayed
	 */
	void updatePixmap(const QPixmap& pixmap);

	/**
	 * @brief Clear the widget
	 */
	void clear();

	/**
	 * @brief Set a text to be displayed in the widget
	 * @param text The text to be displayed
	 */
	void setText(const QString& text);
protected slots:
	void paintEvent(QPaintEvent* event);

private:
	QPixmap mPixmap;
	QString mText;
};

} // end namespace Kopete
#endif
