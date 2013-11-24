#include "buffertest.h"
#include <QtTest/QtTest>
#include "buffer.h"

QTEST_MAIN(BufferTest)

void BufferTest::testAddByte()
{
	Buffer b;
	b.addByte(0x00);
	QVERIFY(b.length() == 1);
	QVERIFY(b.getByte() == 0x00);
}

void BufferTest::testAddWord()
{
	Buffer b;
	b.addWord(0x0101);
	QVERIFY(b.length() == 2);
	QVERIFY(b.getWord() == 0x0101);
}

void BufferTest::testAddDWord()
{
	Buffer b;
	b.addDWord(0x01010101);
	QVERIFY(b.length() == 4);
	QVERIFY(b.getDWord() == 0x01010101);
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

void BufferTest::testBytesAvailable()
{
	Buffer b;
	QVERIFY(b.length() == 0);
	QVERIFY(b.bytesAvailable() == 0);

	b.addByte(0x01);
	b.addWord(0x0203);
	b.addDWord(0x04050607);

	QVERIFY(b.bytesAvailable() == 7);
	b.skipBytes(7);
	QVERIFY(b.bytesAvailable() == 0);
}

void BufferTest::testLength()
{
	Buffer b;
	QVERIFY(b.length() == 0);
	
	b.addByte(0x01);
	b.addWord(0x0203);
	b.addDWord(0x04050607);
	b.addWord(0x0809);

	QVERIFY(b.length() == 9);

	b.getWord();
	QVERIFY(b.length() == 9);
}

void BufferTest::testGuid()
{
	Buffer b;
	Guid g( QByteArray( "asdfghjkqwertyui" ) );
	b.addGuid( g );
	QVERIFY( b.bytesAvailable() == 16 );
	Guid h = b.getGuid();
	QVERIFY( b.bytesAvailable() == 0 );
	QCOMPARE( g, h );
	
}

#include "buffertest.moc"
