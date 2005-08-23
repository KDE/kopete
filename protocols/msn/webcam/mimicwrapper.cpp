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


#include "mimicwrapper.h"

#include "libmimic/mimic.h"

//#include <qbytearray.h>
#include <kdebug.h>
#include <qimage.h>

MimicWrapper::MimicWrapper()  : m_init(false)
{
	m_mimctx=mimic_open();
}

MimicWrapper::~MimicWrapper()
{
	mimic_close(m_mimctx);
}


QPixmap MimicWrapper::decode(const QByteArray& data)
{
	if(!m_init)
	{
		if(!mimic_decoder_init(m_mimctx, (guchar*)(data.data())))
		{
			kdWarning(14140) << k_funcinfo << "Impossible to init decoder" << endl;
			return QPixmap();
		}
		if (!mimic_get_property( m_mimctx, "buffer_size", &m_bufferSize) )
		{
			kdWarning(14140) << k_funcinfo << "Impossible to get buffer size" << endl;
			return QPixmap();
		}
		m_init=true;
	}
	
	QByteArray buff(m_bufferSize);
	if(!mimic_decode_frame(m_mimctx, (guchar*)(data.data()) , (guchar*)(buff.data()) ) )
	{
		kdWarning(14140) << k_funcinfo << "Impossible to decode frame" << endl;
		return QPixmap();
	}
	int width,height;
	mimic_get_property(m_mimctx, "width", &width);
	mimic_get_property(m_mimctx, "height", &height);
	
	
	QByteArray buff2(m_bufferSize*4/3);
	uint b2=0;
	for(uint f=0;f<m_bufferSize;f+=3)
	{
		buff2[b2+0]=buff[f+2];
		buff2[b2+1]=buff[f+1];
		buff2[b2+2]=buff[f+0];
		buff2[b2+3]=0x00;
		b2+=4;
	}

	QImage img( (uchar*)(buff2.data())   , width , height ,  32  , 0L , 0,  QImage::BigEndian  );
	return QPixmap(img);
}

QByteArray MimicWrapper::encode(const QByteArray& data)
{
	if(!m_init)
	{
		if(!mimic_encoder_init(m_mimctx, MIMIC_RES_HIGH))
		{
			kdWarning(14140) << k_funcinfo << "Impossible to init encoder" << endl;
			return QByteArray();
		}
		if (!mimic_get_property( m_mimctx, "buffer_size", &m_bufferSize) )
		{
			kdWarning(14140) << k_funcinfo << "Impossible to get buffer size" << endl;
			return QByteArray();
		}
		m_init=true;
		m_numFrames=0;
	}
	
	QByteArray buff(m_bufferSize);
	int buff_new_size;
	if(!mimic_encode_frame(m_mimctx, (guchar*)(data.data()) , (guchar*)(buff.data()) , (gint*)(&buff_new_size) ,  m_numFrames%15==0  ) )
	{
		kdWarning(14140) << k_funcinfo << "Impossible to decode frame" << endl;
		return QByteArray();
	}
	buff.resize(buff_new_size);
	++m_numFrames;
	return buff;
}
