#ifndef BUFFERTEST_H
#define BUFFERTEST_H

#include <qobject.h>

class BufferTest : public QObject
{
Q_OBJECT
private slots:
	void testAddByte();
	void testAddWord();
	void testAddDWord();
	void testGetTLV();
	void testBytesAvailable();
	void testLength();
	void testGuid();

};

#endif
