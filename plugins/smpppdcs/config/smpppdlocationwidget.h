/*
    smpppdlocationwidget.h

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

#ifndef SMPPPDLOCATIONWIDGET_H
#define SMPPPDLOCATIONWIDGET_H

#include "smpppdlocationui.h"

/**
	@author Heiko Sch&auml;fer <heiko@rangun.de>
*/
class SMPPPDLocationWidget : public SMPPPDLocationWidgetBase
{
	Q_OBJECT

	SMPPPDLocationWidget(const SMPPPDLocationWidget&);
	SMPPPDLocationWidget& operator=(const SMPPPDLocationWidget&);

public:
    SMPPPDLocationWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~SMPPPDLocationWidget();

    void setServer(const QString& serv);
};

#endif
