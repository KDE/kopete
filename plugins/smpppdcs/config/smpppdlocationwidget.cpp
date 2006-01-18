/*
    smpppdlocationwidget.cpp

    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>

    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#include <klineedit.h>

#include "smpppdlocationwidget.h"

SMPPPDLocationWidget::SMPPPDLocationWidget(QWidget* parent, const char* name, WFlags fl)
 : SMPPPDLocationWidgetBase(parent, name, fl) {}

SMPPPDLocationWidget::~SMPPPDLocationWidget() {}

void SMPPPDLocationWidget::setServer(const QString& serv) {
    server->setText(serv);
}

#include "smpppdlocationwidget.moc"
