/*
    avatarfromwebcamdialog.h - Dialog to get a pixmap from a webcam

    Copyright (c) 2009      by Alex Fiestas <alex@eyeos.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETE_AVATARFROMWEBCAMDIALOG_H
#define KOPETE_AVATARFROMWEBCAMDIALOG_H

// KDE includes
#include <kdialog.h>

// Kopete includes
#include <kopete_export.h>

namespace Kopete
{

namespace UI
{

/**
 * @brief Dialog to get a pixmap from the webcam to be set as avatar
 *
 * Using AvatarFromWebcamDialog is very simple
 *
 * @code
Kopete::UI::AvatarFromWebcamDialog *dialog = new Kopete::UI::AvatarFromWebcamDialog();
int result = dialog->exec();
if( result == KDialog::Ok ) {
   QPixmap pixmap = dialog->getLastPixmap();
}
 * @endcode
 *
 * @author Alex Fiestas <alex@eyeos.org>
 */
class KOPETE_EXPORT AvatarWebcamDialog : public KDialog
{
	Q_OBJECT
public:
	/**
	 * Create a new AvatarFromWebcamDialog
	 * @param parent Parent widget
	 */
	AvatarWebcamDialog(QWidget *parent = 0);
	/**
	 * Clean-up resource of AvatarFromWebcamDialog
	 */
	virtual ~AvatarWebcamDialog();
	/**
	 * Return the last captured pixmap
	 */
	QPixmap& getLastPixmap();

private Q_SLOTS:
	/**
	*Internal use, updates the webcamwidget image
	*/
	void updateImage();

protected Q_SLOTS:

	/**
	 * @internal
	 * A button has been clicked. Reimplemented from @ref KDialog::slotButtonClicked()
	 */
	virtual void slotButtonClicked(int button);

private:
	class Private;
	Private * const d;
};

} // namespace UI

} // namespace Kopete
#endif

