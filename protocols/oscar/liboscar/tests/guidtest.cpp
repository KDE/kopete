
#include <QtTest/QtTest>
#include "guidtest.h"
#include "oscarguid.h"

QTEST_MAIN( GuidTest )

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
