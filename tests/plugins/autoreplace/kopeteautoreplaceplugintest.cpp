/*
    Tests for Kopete AutoReplace Plugin

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
#include "kopetechatsessionmanager.h"
#include "kopetemessage.h"
#include "autoreplaceplugin.h"
#include "autoreplaceconfig.h"

#include <QObject>
#include <QtTest>
Q_DECLARE_METATYPE(Kopete::Message);
Q_DECLARE_METATYPE(Kopete::Message::MessageDirection);


class AutoReplacePluginTest : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultAutoReplace();
    void testAutoReplaceConfig();
    void testDefaultAutoReplace_data();
};

void AutoReplacePluginTest::testDefaultAutoReplace_data() {
    QTest::addColumn<QString>("messageTest");
    QTest::addColumn<QString>("messageExpected");
    QTest::addColumn<Kopete::Message::MessageDirection>("direction");
    QTest::newRow("Unaffected Outbound") << QStringLiteral("hello") << QStringLiteral("hello") << Kopete::Message::Outbound;
    QTest::newRow("Empty Outbound") << QString() << QString() << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 1") << QStringLiteral("arent") << QStringLiteral("are not") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 2") << QStringLiteral("r") << QStringLiteral("are") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 3") << QStringLiteral("ur") << QStringLiteral("your") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 4") << QStringLiteral("u") << QStringLiteral("you") << Kopete::Message::Outbound;
    QTest::newRow("Unaffected Inbound") << QStringLiteral("hello") << QStringLiteral("hello") << Kopete::Message::Inbound;
    QTest::newRow("Empty Inbound") << QString() << QString() << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 1") << QStringLiteral("arent") << QStringLiteral("arent") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 2") << QStringLiteral("r") << QStringLiteral("r") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 3") << QStringLiteral("ur") << QStringLiteral("ur") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 4") << QStringLiteral("u") << QStringLiteral("u") << Kopete::Message::Inbound;
}

void AutoReplacePluginTest::testDefaultAutoReplace()
{
    QFETCH(QString, messageExpected);
    QFETCH(QString, messageTest);
    QFETCH(Kopete::Message::MessageDirection, direction);

    AutoReplacePlugin *A = new AutoReplacePlugin(nullptr, QVariantList());
    Kopete::ChatSessionManager *csm = Kopete::ChatSessionManager::self();
    QSignalSpy spy(csm, &Kopete::ChatSessionManager::aboutToSend);
    
    Kopete::Message msg;
    msg.setPlainBody(messageTest);  
    msg.setDirection(direction);
    QVERIFY(spy.isValid());
    emit csm->aboutToSend(msg);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(msg.plainBody(), messageExpected);
    delete A;
    delete csm;
}

void AutoReplacePluginTest::testAutoReplaceConfig()
{
    AutoReplaceConfig *config = new AutoReplaceConfig();
    QVERIFY(!config->autoReplaceIncoming());
    QVERIFY(config->autoReplaceOutgoing());
    QVERIFY(!config->dotEndSentence());
    QVERIFY(!config->capitalizeBeginningSentence());
    config->setAutoReplaceIncoming(true);
    QVERIFY(config->autoReplaceIncoming());
    config->setDotEndSentence(true);
    QVERIFY(config->dotEndSentence());
    config->setCapitalizeBeginningSentence(true);
    QVERIFY(config->capitalizeBeginningSentence());
    AutoReplaceConfig::WordsToReplace map = config->map();
    const QString value = QStringLiteral("Hello Auto Replace Plugin");
    const QString key = QStringLiteral("Are not.");
    map[key] = value;
    config->setMap(map);
    AutoReplaceConfig::WordsToReplace newMap = config->map();
    QCOMPARE(newMap[key], map[key]);
    delete config;
}

QTEST_MAIN(AutoReplacePluginTest)
#include "kopeteautoreplaceplugintest.moc"
