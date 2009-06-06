/*
    qcatlshandler.h - Kopete Groupwise Protocol
  
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>
    
    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
  
#ifndef GWQCATLSHANDLER_H
#define GWQCATLSHANDLER_H

//#include <qtimer.h>
#include "libgroupwise_export.h"
#include "tlshandler.h"

namespace QCA
{
class TLS;
}

class LIBGROUPWISE_EXPORT QCATLSHandler : public TLSHandler
{
	Q_OBJECT
public:
	QCATLSHandler(QCA::TLS *parent);
	~QCATLSHandler();

	QCA::TLS *tls() const;
	int tlsError() const;

	void reset();
	void startClient(const QString &host);
	void write(const QByteArray &a);
	void writeIncoming(const QByteArray &a);

signals:
	void tlsHandshaken();

public slots:
	void continueAfterHandshake();

private slots:
	void tls_handshaken();
	void tls_readyRead();
	void tls_readyReadOutgoing();
	void tls_closed();
	void tls_error();

private:
	class Private;
	Private * const d;
};

#endif
