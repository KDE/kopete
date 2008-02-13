/*
    applicationWidget.h -

    Copyright (c) 2007-2008 by Romain Castan      <romaincastan@gmail.com>
    Copyright (c) 2007-2008 by Bertrand Demay     <bertranddemay@gmail.com>
    Copyright (c) 2007-2008 by Julien Hubatzeck   <reineur31@gmail.com>
    Copyright (c) 2007-2008 by Michel Saliba      <msalibaba@gmail.com>

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

#ifndef _APPLICATIONWIDGET_H_
#define _APPLICATIONWIDGET_H_

#include "ui_mainWindow.h"
#include <QApplication>
#include <QtGui>
#include <QTextEdit>
#include <QString>
#include <QTreeWidget>
 #include <QLineEdit>

#include "../upnpRouter.h"

class ApplicationWidget : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT
 
public:
	ApplicationWidget(QWidget *parent = 0);
	UPnpRouter router;

public slots:
	void openPort();
	void deletePort();

};

#endif
