/*
  aim.cpp  -  Oscar Protocol Plugin

  Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

  This file is based on aim.h from Kit client.

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#include "aim.h"
#include <qstring.h>
#include <qregexp.h>
QString tocNormalize(const QString &oldstr)
{
	return oldstr.lower().replace(" ","");
}
