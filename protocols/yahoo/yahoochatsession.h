/*
    yahoochatsession.h - Yahoo! Message Manager

    Copyright (c) 2005 by Andre Duffeck        <duffeck@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOCHATSESSION_H
#define YAHOOCHATSESSION_H

#include "kopetechatsession.h"
#include "kopete_export.h"

class QLabel;


/**
 * @author Andre Duffeck
 */
class YAHOO_EXPORT YahooChatSession : public Kopete::ChatSession
{
	Q_OBJECT

public:
	YahooChatSession( Kopete::Protocol *protocol, const Kopete::Contact *user, Kopete::ContactPtrList others );
	~YahooChatSession();

private slots:
	void slotDisplayPictureChanged();

	void slotBuzzContact();
	void slotUserInfo();
	void slotRequestWebcam();
	void slotInviteWebcam();
	void slotSendFile();

private:
	QLabel *m_image;
};

#endif
