/*
    icqeditaccountwidget.h - ICQ Account Widget

    Copyright (c) 2003 by Chris TenHarmsel  <tenharmsel@staticmethod.net>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#ifndef ICQEDITACCOUNTWIDGET_H
#define ICQEDITACCOUNTWIDGET_H

#include <qwidget.h>
#include "editaccountwidget.h"
/**
 * @author Chris TenHarmsel <tenharmsel@staticmethod.net>
 */

class KopeteAccount;

class ICQProtocol;
class OscarEditAccountUI;

class ICQEditAccountWidget : public QWidget, public EditAccountWidget
{
    Q_OBJECT
public:
    /** Constructor */
    ICQEditAccountWidget(ICQProtocol *protocol, KopeteAccount *account,
			   QWidget *parent=0, const char *name=0);
    /** Destructor */
    virtual ~ICQEditAccountWidget();
    /** Validates the input */
    virtual bool validateData();
    /** Applies the data */
    virtual KopeteAccount *apply();
protected:
    /** Our account we're editing */
    KopeteAccount *mAccount;
    /** The Protocol we're in */
    ICQProtocol *mProtocol;
    /** The GUI */
    OscarEditAccountUI *mGui;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
