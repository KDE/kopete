/*
   Kopete Oscar Protocol
   xtrazxrandomizerservice.h - Xtraz XRandomizerService

   Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

   Kopete (c) 2007 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#ifndef XTRAZXRANDOMIZERSERVICE_H
#define XTRAZXRANDOMIZERSERVICE_H

#include "xtrazxservice.h"

namespace Xtraz
{

class XRandomizerService : public XService
{
public:
	XRandomizerService();

	virtual QString serviceId() const;
};

}

#endif

