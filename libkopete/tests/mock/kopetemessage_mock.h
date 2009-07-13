/*
    Message mock object class

    Copyright (c) 2005 by Duncan Mac-Vicar Prett  <duncan@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _KOPETEMESSAGE_MOCK_H_
#define _KOPETEMESSAGE_MOCK_H_

#include "kopetemessage.h"

namespace Kopete
{
namespace Test
{
namespace Mock
{

class Message : public Kopete::Message
{

};

} // end ns Kopete::Test::Mock
} // end ns Kopete::Test
} // end ns Kopete

#endif
