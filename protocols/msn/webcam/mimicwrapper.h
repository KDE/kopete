/*
    Copyright (c) 2005 by Olivier Goffart        <ogoffart@ kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef MIMICWRAPPER_H
#define MIMICWREPPER_H

#include <qpixmap.h>

#include "kopete_export.h"

typedef struct _MimCtx MimCtx;

class KOPETE_EXPORT MimicWrapper
{
	public:
		MimicWrapper();
		~MimicWrapper();
		
		QPixmap decode(const QByteArray &data);
		QByteArray encode(const QByteArray &data);
		
	private:
		MimCtx *m_mimctx;
		bool m_init;
		uint m_bufferSize;
		uint m_numFrames;
};

#endif

