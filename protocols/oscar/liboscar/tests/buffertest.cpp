#include <QtTest/QtTest>
#include "buffertest.h"
#include "buffer.h"

QTEST_MAIN(BufferTest)

void BufferTest::testAddByte()
{
	Buffer b;
	b.addByte(0x00);
	QVERIFY(b.length() == 1);
	QVERIFY(b.getByte() == 0x00);
}

void BufferTest::testGetTLV()
{
	char raw[] = { 0x00, 0x00, 0x00, 0x01 };
	Buffer b;
	b.addDWord(0x00030004);
	b.addDWord(0x00000001);

	TLV t = b.getTLV();

	QVERIFY( t.type == 0x0003 );
	QVERIFY( t.length == 0x0004 );
	QVERIFY( t.data == QByteArray::fromRawData( raw, 4 ) );
}
#include "buffertest.moc"
