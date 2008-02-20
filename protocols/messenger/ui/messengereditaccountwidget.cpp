/*
    messengereditaccountwidget.cpp - Windows Live Messenger Account Widget

	Copyright (c) 2007		by Zhang Panyong		<pyzhang@gmail.com>
    Copyright (c) 2003      by Olivier Goffart       <ogoffart@kde.org>
    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "messengereditaccountwidget.h"

#include <QCheckBox>
#include <q3groupbox.h>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QLatin1String>
#include <QWidget>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <ktoolinvocation.h>
#include <kconfig.h>
#include <kpixmapregionselectordialog.h>
#include <kconfiggroup.h>

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "kopetepasswordwidget.h"

#include "messengeraccount.h"
#include "messengercontact.h"
#include "ui_messengereditaccount.h"
/*#include "msnnotifysocket.h"*/
#include "messengerprotocol.h"

// TODO: This was using KAutoConfig before, use KConfigXT instead.
class MessengerEditAccountWidgetPrivate
{
public:
	MessengerProtocol *protocol;
	Ui::MessengerEditAccountUI *ui;

	QString pictureUrl;
	QImage pictureData;
};

// MessengerEditAccountWidget::MessengerEditAccountWidget( Kopete::Account *account )
// : QWidget(0 ), KopeteEditAccountWidget( account )
// {
// }

// MessengerEditAccountWidget::~MessengerEditAccountWidget()
// {
// 	//delete d;
// }

Kopete::Account * MessengerEditAccountWidget::apply()
{
	return 0;
}

/*check if it's the validate Windows Live ID */
bool MessengerEditAccountWidget::validateData()
{
	return false;
}

void MessengerEditAccountWidget::slotAllow()
{
}

void MessengerEditAccountWidget::slotBlock()
{
}

/*show reverse list*/
void MessengerEditAccountWidget::slotShowReverseList()
{
}

void MessengerEditAccountWidget::slotSelectImage()
{
}

void MessengerEditAccountWidget::slotOpenRegister()
{
}

//#include "messengereditaccountwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

