/*
   ProgressBarSample.h -

    Copyright (c) 2007-2008 by Kevin KIN-FOO      <kevinkinfoo@gmail.com>

    Kopete    (c) 2002-2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef _PROGRESSBAR_H_
#define _PROGRESSBAR_H_

#include "ui_progressTests.h"

#include <QApplication>
#include <QtGui>
#include <QString>
#include <QProgressBar>
#include <QAbstractButton>

class ProgressBarSample : public QDialog, private Ui::Main
{
	Q_OBJECT
public:
	// Constructor
	ProgressBarSample(QWidget *parent = 0);

public slots:
	void launch();
};
#endif
