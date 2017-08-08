/*
    Tests for Kopete TextEffect Plugin

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
#include "texteffectplugin.h"
#include "texteffectconfig.h"

#include <KSharedConfig>
#include <KConfigGroup>
#include <QObject>
#include <QtTest>
Q_DECLARE_METATYPE(Kopete::Message);
Q_DECLARE_METATYPE(Kopete::Message::MessageDirection);


class TextEffectPluginTest : public QObject
{
    Q_OBJECT
private slots:
    void testDefaultTextEffect();
    void testTextEffectConfig();
    void testDefaultTextEffect_data();
    void testTextEffectConfig_data();
};

void TextEffectPluginTest::testDefaultTextEffect_data() 
{
    QTest::addColumn<QString>("messageTest");
    QTest::addColumn<QString>("colorExpected");
    QTest::addColumn<Kopete::Message::MessageDirection>("direction");
    QTest::newRow("Ideal Outbound Case") << QStringLiteral("a>\n \n") << QStringLiteral("#0088dd") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 1") << QString() << QString("#0088dd") << Kopete::Message::Outbound;    
    QTest::newRow("Affected Case 2") << QStringLiteral("<") << QStringLiteral("#0088dd") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 3") << QStringLiteral("&") << QStringLiteral("#0088dd") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 4") << QStringLiteral("\n") << QStringLiteral("#0088dd") << Kopete::Message::Outbound;
    QTest::newRow("Ideal Inbound Case") << QStringLiteral(">") << QStringLiteral("#000000") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 1") << QString() << QString("#000000") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 2") << QStringLiteral("<") << QStringLiteral("#000000") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 3") << QStringLiteral("&") << QStringLiteral("#000000") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 4") << QStringLiteral("\n") << QStringLiteral("#000000") << Kopete::Message::Inbound; 
}

void TextEffectPluginTest::testDefaultTextEffect()
{
    QFETCH(QString, colorExpected);
    QFETCH(QString, messageTest);
    QFETCH(Kopete::Message::MessageDirection, direction);
    
    TextEffectPlugin *pulgin = new TextEffectPlugin(nullptr, QVariantList());
    Kopete::ChatSessionManager *csm = Kopete::ChatSessionManager::self();
    QSignalSpy spy(csm, &Kopete::ChatSessionManager::aboutToSend);
    TextEffectConfig *config = new TextEffectConfig();
    QStringList colorList = config->defaultColorList();
    Kopete::Message msg;
    msg.setPlainBody(messageTest);  
    msg.setDirection(direction);
    emit csm->aboutToSend(msg);
    QCOMPARE(msg.foregroundColor().name(QColor::HexRgb), colorExpected);
    delete plugin;
    delete config;
}

void TextEffectPluginTest::testTextEffectConfig_data() 
{
    QTest::addColumn<QString>("messageTest");
    QTest::addColumn<QString>("messageExpected");
    QTest::addColumn<Kopete::Message::MessageDirection>("direction");
    QTest::newRow("Ideal Outbound Case") << QStringLiteral("asdfghjkl") << QStringLiteral("AsDfGhJkL") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 1") << QString() << QString() << Kopete::Message::Outbound;    
    QTest::newRow("Affected Case 2") << QStringLiteral("/") << QStringLiteral("/") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 3") << QStringLiteral(".AsDfghjkl") << QStringLiteral(".aSdFgHjKl") << Kopete::Message::Outbound;
    QTest::newRow("Affected Case 4") << QStringLiteral("aSdF") << QStringLiteral("AsDf") << Kopete::Message::Outbound;
    QTest::newRow("UnAffected Case 1") << QStringLiteral("***") << QStringLiteral("***") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 2") << QString() << QString() << Kopete::Message::Inbound;
    QTest::newRow("Ideal Outbound Case") << QStringLiteral("rome") << QStringLiteral("rome") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 3") << QStringLiteral("123456") << QStringLiteral("123456") << Kopete::Message::Inbound;
    QTest::newRow("UnAffected Case 4") << QStringLiteral("/") << QStringLiteral("/") << Kopete::Message::Inbound; 
}

void TextEffectPluginTest::testTextEffectConfig()
{
    QFETCH(QString, messageExpected);
    QFETCH(QString, messageTest);
    QFETCH(Kopete::Message::MessageDirection, direction);
    
    TextEffectConfig *config = new TextEffectConfig();
    config->load();
    config->setColorWords(true);
    config->save();
    config->setColorWords(false);
    config->load(); // value should be overridden
    QVERIFY(config->colorWords());
    config->setColorWords(false); // reset as we are going to test waves
    config->save();
    TextEffectPlugin *plugin = new TextEffectPlugin(nullptr, QVariantList());
    config->setWaves(true);
    config->save();
    plugin->slotSettingsChanged();
    Kopete::ChatSessionManager *csm = Kopete::ChatSessionManager::self();
    Kopete::Message msg;
    msg.setPlainBody(messageTest);  
    msg.setDirection(direction);
    emit csm->aboutToSend(msg);
    QCOMPARE(msg.plainBody(), messageExpected);
    KConfigGroup configr(KSharedConfig::openConfig(), "TextEffect Plugin");
    configr.deleteGroup();
    delete plugin;
    delete csm;
    delete config;
}

QTEST_MAIN(TextEffectPluginTest)
#include "kopetetexteffectplugintest.moc"
