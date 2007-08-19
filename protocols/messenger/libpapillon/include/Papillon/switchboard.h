/*
    switchboard.h - Messenger Switchboard handle class

    Copyright (c) 2007		by Zhang Panyong        <pyzhang@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef PAPILLONSWITCHBOARD_H
#define PAPILLONSWITCHBOARD_H
namespace Papillon
{
class PAPILLON_EXPORT SwitchBoard : public QObject
{
	Q_OBJECT
public:
	explicit SwitchBoard(Client *client);

	~SwitchBoard();

	Client * client();

	setServer(const QString &server, uint port);

private:
	class Private;
	Private *d;
}

}

#endif/* PAPILLONSWITCHBOARD_H*/

