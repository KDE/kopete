/*
    ligprotocol.h - Kopete Lig Protocol

    Copyright (c) 2007      by Cláudio da Silveira Pinheiro	<taupter@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef LIGPROTOCOL_H
#define LIGPROTOCOL_H

#include "kopeteprotocol.h"

/**
	@author Kopete Developers <kopete-devel@kde.org>
*/
class LigProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
    LigProtocol();

    ~LigProtocol();

};

#endif
