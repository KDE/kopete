
#include "guidtest.h"
#include <QtTest/QtTest>
#include "oscarguid.h"

QTEST_MAIN( GuidTest )

void GuidTest::testConstructors()
{
	QByteArray id = "0123456789abcdef";
	Oscar::Guid g(id);
	QVERIFY( g.isValid() );
	QCOMPARE( g.data(), id );
	Oscar::Guid h( QLatin1String( "0a-1b-2c" ) );
	QVERIFY( ! h.isValid() );
	Oscar::Guid i( QLatin1String( "30313233--3435-3637-3839-616263646566" ) );
	QVERIFY( i.isValid() );
	QCOMPARE( i.data(), id );
	Oscar::Guid j( QByteArray::fromRawData( "Kopete ICQ \0\xc\0\0", 16 ) );
	QVERIFY( j.isValid() );
}

void GuidTest::testSetData()
{
	QByteArray id = "0123456789abcdef";
	QVERIFY( id.length() == 16 );
	Oscar::Guid g;
	QVERIFY( ! g.isValid() );
	g.setData(id);
	QVERIFY( g.isValid() );
	QCOMPARE( g.data(), id );
}

void GuidTest::testIsVaild()
{
	Oscar::Guid g;
	QVERIFY( ! g.isValid() );
	g.setData( QByteArray( "" ) );
	QVERIFY( ! g.isValid() );
	g.setData( QByteArray( "asdfghjkqwertyu" ) );
	QVERIFY( ! g.isValid() );
	g.setData( QByteArray( "asdfghjkqwertyui" ) );
	QVERIFY( g.isValid() );
	g.setData( QByteArray( "asdfghjkqwertyuio" ) );
	QVERIFY( ! g.isValid() );

}

void GuidTest::testCompare()
{
	Oscar::Guid g( QByteArray( "asdfghjkqwertyui" ) );
	Oscar::Guid h( g );
	Oscar::Guid i( QByteArray( "asdfghjkqwertyui" ) );
	Oscar::Guid j( QByteArray( "qwertyuiasdfghjk" ) );

	QCOMPARE( g, h );
	QCOMPARE( g, i );
	QCOMPARE( h, i );
	QVERIFY( !( j == g ) ); //there is no !=

}


#include "guidtest.moc"
