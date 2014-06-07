/*
    libjinglecalldialog.cpp - libjingle support

    Copyright (c) 2009-2014 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "libjinglecalldialog.h"

#include <QCloseEvent>

LibjingleCallDialog::LibjingleCallDialog(QWidget *parent): QDialog(parent) {
	setupUi(this);
}

void LibjingleCallDialog::closeEvent(QCloseEvent * e) {
	e->ignore();
	emit(closed());
}

#include "libjinglecalldialog.moc"

