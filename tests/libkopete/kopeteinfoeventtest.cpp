/*
    Tests for Kopete::InfoEvent

    Copyright (c) 2017      by Vijay Krishnavanshi    <vijaykrishnavashi>

    Kopete    (c) 2002-2017 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/


#include "kopeteinfoevent.h"

#include <QObject>
#include <QtTest>
class InfoEventTest : public QObject
{
    Q_OBJECT
private slots:
    void testClassProperties();
    void testClassProperties_data();
};

void InfoEventTest::testClassProperties_data()
{
    QTest::addColumn<QString>("Title");
    QTest::addColumn<QString>("Text");
    QTest::addColumn<QString>("AdditionalText");
    QTest::addColumn<QString>("ActionText");

    QTest::newRow("Ideal Example") << QStringLiteral("Title") << QStringLiteral("Text") << QStringLiteral("AdditionalText") << QStringLiteral("ActionText");
}

void InfoEventTest::testClassProperties()
{
    Kopete::InfoEvent *event = new Kopete::InfoEvent(nullptr);
    QFETCH(QString, Title);
    QFETCH(QString, Text);
    QFETCH(QString, AdditionalText);
    uint ActionId = 125;
    QFETCH(QString, ActionText);
    QSignalSpy spy(event, &Kopete::InfoEvent::changed);
    QVERIFY(spy.isValid());
    event->setTitle(Title);
    QCOMPARE(event->title(), Title);
    QCOMPARE(spy.count(), 1);
    event->setText(Text);
    QCOMPARE(event->text(), Text);
    QCOMPARE(spy.count(), 2);
    event->setAdditionalText(AdditionalText);
    QCOMPARE(event->additionalText(), AdditionalText);
    QCOMPARE(spy.count(), 3);
    event->addAction(ActionId, ActionText);
    QCOMPARE(spy.count(), 4);
    QMap<uint, QString> map = event->actions();
    QCOMPARE(map[ActionId], ActionText);
    QSignalSpy spyActivate(event, &Kopete::InfoEvent::actionActivated);
    QVERIFY(spyActivate.isValid());
    event->activate(ActionId);
    QCOMPARE(spyActivate.count(), 1);
    qRegisterMetaType<uint>();
    uint temp = qvariant_cast<uint>(spyActivate.at(0).at(0));
    QCOMPARE(temp, ActionId);
    QSignalSpy spyClosed(event, &Kopete::InfoEvent::eventClosed);
    QVERIFY(spyClosed.isValid());
    event->close();
    QVERIFY(event->isClosed());
    QCOMPARE(spyClosed.count(), 1);
}

QTEST_MAIN(InfoEventTest)
#include "kopeteinfoeventtest.moc"
