#ifndef BUFFERTEST_H
#define BUFFERTEST_H

#include <qobject.h>

class BufferTest : public QObject
{
Q_OBJECT
private slots:
	void testAddByte();
	void testGetTLV();

};

#endif
