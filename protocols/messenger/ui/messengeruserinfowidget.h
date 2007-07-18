/*
    messengeruserinfowidget.h: Messenger User Info widget header

    Copyright (c) 2007 by pyzhang <pyzhang@gmail.com>
	Kopete    (c) 2002-2007 by The Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _MESSENGERUSERINFOWIDGET_H_
#define _MESSENGERUSERINFOWIDGET_H_

#include <kpagedialog.h>

class MessengerUserInfoWidget : public KPageDialog
{
Q_OBJECT
public:
	MessengerUserInfoWidget( QWidget* parent = 0);
	~MessengerUserInfoWidget();

private:
	Ui::MessengerGeneralInfoWidget* m_genInfoWidget;
	Ui::MessengerContactInfoWidget* m_contactInfoWidget;
	Ui::MessengerPersonalInfoWidget* m_personalInfoWidget;
	Ui::MessengerWorkInfoWidget* m_workInfoWidget;
	Ui::MessengerNotesInfoWidget* m_notesInfoWidget;

private slot:
	void slotUpdateNickName();
}

#endif
