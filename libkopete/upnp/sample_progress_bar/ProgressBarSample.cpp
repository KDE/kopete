/*
    ProgressBarSample.cpp -

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
#include <iostream>
#include <QDebug>

// Connect runTestButton to the code of launch
ProgressBarSample::ProgressBarSample(QWidget *parent): QDialog(parent) 
{
	setupUi((QDialog *)this);
	connect(runTestButton, SIGNAL(clicked()),(QDialog *)this, SLOT(launch()));
}

// Code related with the the click on runTestButton
void ProgressBarSample::launch() {
	testsBar->setValue(	(testsBar->value() < testsBar->maximum()) 
				? testsBar->value()+1
				: testsBar->minimum());
	qDebug() << "In ProgressBarSample::launch()"
		<< endl;
}
