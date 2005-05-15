/***************************************************************************
                          translatordialog.h  -  description
                             -------------------
    begin                : sam oct 19 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart @ kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSLATORDIALOG_H
#define TRANSLATORDIALOG_H

#include <qwidget.h>
#include <kdialogbase.h>

//#include <kopetemessage.h>

class KTextEdit;

/**
 * @author Olivier Goffart
 */
class TranslatorDialog : public KDialogBase
{
	Q_OBJECT

public:
	TranslatorDialog(const QString &translated, QWidget *parent=0, const char *name=0);
	~TranslatorDialog();

	QString translatedText();

private:
	KTextEdit *m_textEdit;

private slots: // Public slots
//  void slotFinished();
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

