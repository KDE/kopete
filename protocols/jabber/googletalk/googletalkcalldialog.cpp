/*
    googletalkcalldialog.cpp - Google Talk and Google libjingle support

    Copyright (c) 2009 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "googletalkcalldialog.h"

#include <QCloseEvent>

GoogleTalkCallDialog::GoogleTalkCallDialog(QWidget *parent): QDialog(parent) {
	setupUi(this);
}

void GoogleTalkCallDialog::closeEvent(QCloseEvent * e) {
	e->ignore();
	emit(closed());
}

#include "googletalkcalldialog.moc"

