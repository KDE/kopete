/*
    translatordialog.h  -  description

	Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#ifndef TRANSLATORDIALOG_H
#define TRANSLATORDIALOG_H

#include <qwidget.h>
#include <kdialog.h>

//#include <kopetemessage.h>

class KTextEdit;

/**
 * @author Olivier Goffart
 */
class TranslatorDialog : public KDialog
{
	Q_OBJECT

public:
	explicit TranslatorDialog(const QString &translated, QWidget *parent=0);
	~TranslatorDialog();

	QString translatedText();

private:
	KTextEdit *m_textEdit;

private slots: // Public slots
//  void slotFinished();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

