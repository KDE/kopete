/*
    main.cpp -

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
#include "ProgressBarSample.h"
#include <QApplication>

int main ( int argc, char* argv[]){
	// Building the application
	QApplication app(argc, argv);
	// Retrieve our interface
	Ui_Main ui;
	QDialog *window = new QDialog;
	ui.setupUi(window);
	// Show the window
	window->show();
	return app.exec();
}
