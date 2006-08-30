/*
    File Transfer Test

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

#ifndef FILETRANSFERTEST_H
#define FILETRANSFERTEST_H

#include "oscartestbase.h"

class FileTransferTest : public OscarTestBase
{
Q_OBJECT
private slots:
	void testRRequest();
};

#endif
