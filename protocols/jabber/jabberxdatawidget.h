 /*
    Copyright (c) 2008 by Igor Janssen  <alaves17@gmail.com>

    Kopete    (c) 2008 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */

#ifndef JABBERXDATAWIDGET_H
#define JABBERXDATAWIDGET_H

#include <QWidget>
#include <QLayout>
#include "xmpp_xdata.h"

class XDataWidgetField;

class JabberXDataWidget : public QWidget
{
	Q_OBJECT
public:
	explicit JabberXDataWidget(const XMPP::XData &data, QWidget *parent = 0);
	~JabberXDataWidget();

	XMPP::XData::FieldList fields() const;

	bool isValid() const;

private:
	QList<XDataWidgetField *> mFields;
};

#endif
