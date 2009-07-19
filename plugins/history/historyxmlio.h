/*
    historyxmlio.h

    Copyright (c) 2009 by Kaushik Saurabh        <roideuniverse@gmailcom>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef HISTORYXMLIO_H
#define HISTORYXMLIO_H

class History;

class QIODevice;



class HistoryXmlIo
{
public:


  
  static bool writeHistoryToXml( const History &history, QIODevice *device );

  static bool readHistoryFromXml( QIODevice *device, History &history );

  static bool writeHistoryHeaderToXml( const History &history, QIODevice *device );
 
  static bool readHistoryHeaderFromXml( QIODevice *device, History &history );
 
  static bool writeHistoryMessagesToXml( const History &history, QIODevice *device );
 
  static bool readHistoryMessagesFromXml( QIODevice *device, History &history );

};

#endif