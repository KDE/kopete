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


class AutoReplacePluginTest : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultAutoReplace();
    void testAutoReplaceConfig();
};

void AutoReplacePluginTest::testDefaultAutoReplace()
{
    Kopete::Message msg;
    msg.setPlainBody("arent");  
    msg.setDirection(Kopete::Message::Outbound);
    AutoReplacePlugin *A = new AutoReplacePlugin(nullptr, QVariantList());
    Kopete::ChatSessionManager *csm = Kopete::ChatSessionManager::self();
    QSignalSpy spy(csm, &Kopete::ChatSessionManager::aboutToSend);
    QVERIFY(spy.isValid());
    emit csm->aboutToSend(msg);
    QCOMPARE(msg.plainBody(), QStringLiteral("are not"));
    msg.setPlainBody("u");
    emit csm->aboutToSend(msg);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(msg.plainBody(), QStringLiteral("you"));
    msg.setPlainBody("r");
    emit csm->aboutToSend(msg);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(msg.plainBody(), QStringLiteral("are"));
    msg.setPlainBody("ur");
    emit csm->aboutToSend(msg);
    QCOMPARE(spy.count(), 4);
    QCOMPARE(msg.plainBody(), QStringLiteral("your"));
    Q_UNUSED(A);
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
}

QTEST_MAIN(AutoReplacePluginTest)
#include "kopeteautoreplaceplugintest.moc"
