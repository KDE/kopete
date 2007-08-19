/*
   offlinemessage.h - Papillon OfflineMessage to Windows Live Messenger.
 
   Copyright (c) 2007		by Zhang Panyong  <pyzhang@gmail.com>
   Kopete    (c) 2002-2005 by the Kopete developers	<kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONOFFLINEMESSAGE_H
#define PAPILLONOFFLINEMESSAGE_H
namespace Papillon
{

class PAPILLON_EXPORT OfflineMessage : public QObject
{
	Q_OBJECT
public:
	explicit OfflineMessage(Client *client);

	~OfflineMessage();

	Client * client();

private:
	class Private;
	Private *d;
}

}
#endif
