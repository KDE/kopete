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