/*
    Connection Test

    Copyright (c) 2006 by Matt Rogers <mattr@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CONNECTIONTEST_H
#define CONNECTIONTEST_H

#include "oscartestbase.h"

class ConnectionTest : public OscarTestBase
{
Q_OBJECT
private Q_SLOTS:
    ///Checks to make sure that using the non list form of 
    ///addToSupportedFamilies works correctly
    void checkSupportedFamiliesSingle();

    ///Checks to make sure that using the list form of addToSupportedFamilies
    ///works correctly
    void checkSupportedFamiliesList();
};

#endif
