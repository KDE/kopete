/*
   yahoomessagetask.h - Windows Live Messenger Yahoo Message Task

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
#ifndef PAPILLONYAHOOMESSAGETASK_H
#define PAPILLONYAHOOMESSAGETASK_H

#include <Papillon/Task>
namespace Papillon 
{

class PAPILLON_EXPORT YahooMessageTask : public Papillon::Task
{
	Q_OBJECT
public:
	explicit YahooMessageTask(Papillon::Task *parent);

	virtual ~YahooMessageTask();

	virtual bool take(Papillon::NetworkMessage *networkMessage);

private:
	class Private;
	Private *d;
}

}
#endif
