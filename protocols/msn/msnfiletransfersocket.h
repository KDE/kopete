/***************************************************************************
                          msnfiletransfersocket.h  -  description
                             -------------------
    begin                : mer jui 31 2002
    copyright            : (C) 2002 by Olivier Goffart
    email                : ogoffart@tiscalinet.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MSNFILETRANSFERSOCKET_H
#define MSNFILETRANSFERSOCKET_H

#include <qwidget.h>
#include "msnsocket.h"

/**
  *@author Olivier Goffart
  */

class QFile;
class KopeteTransfer;


class MSNFileTransferSocket : public MSNSocket  {
   Q_OBJECT
public:
	MSNFileTransferSocket(const QString msnid, const QString cook, const QString filename, QObject* parent=0L);
	~MSNFileTransferSocket();
  /** No descriptions */
  void setKopeteTransfer(KopeteTransfer *kt);
protected: // Protected methods
  /**
	 * This reimplementation sets up the negotiating with the server and
	 * suppresses the change of the status to online until the handshake
	 * is complete.
	 */
  virtual void doneConnect();
  /**
	 * Handle an MSN command response line.
	 */
  virtual void parseCommand(const QString & cmd, uint id, const QString & data);
  /** No descriptions */
  void bytesReceived(const QByteArray & data);
  /**
	 * Check if we're waiting for a block of raw data. Emits blockRead()
	 * when the data is available.
	 * Returns true when still waiting and false when there is no pending
	 * read, or when the read is succesfully handled.
	 */



private:
  QString m_msnId;
  QString m_authcook;
  long unsigned int m_size;
  long unsigned int m_downsize;
  QString m_fileName;

  KopeteTransfer* m_kopeteTransfer;

  QFile *m_file ;


private slots: // Private slots
  /** No descriptions */
  void slotSocketClosed();
  /** No descriptions */
  void slotReadBlock(const QByteArray &);
};

#endif
