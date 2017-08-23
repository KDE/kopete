/*
    Tests for LibKopete - AddressBookLinkWidget UI

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
#include "ui_addressbooklinkwidget_base.h"

#include <QObject>
#include <QtTest>
class AddressBookLinkWidget : public QObject
{
    Q_OBJECT
private slots:
    void testButtons_data();
    void testButtons();
    void testLabel_data();
    void testLabel();
};

void AddressBookLinkWidget::testButtons_data()
{
    QTest::addColumn<QTestEventList>("events");
    QTest::addColumn<int>("expected");

    QTestEventList list1;
    list1.addMouseClick(Qt::LeftButton);
    list1.addMouseClick(Qt::LeftButton);
    list1.addMouseClick(Qt::LeftButton);
    list1.addMouseClick(Qt::LeftButton);
    QTest::newRow("without delay") << list1 << 4;

    QTestEventList list2;
    list2.addMouseClick(Qt::LeftButton);
    list2.addMouseClick(Qt::LeftButton);
    list2.addDelay(50);
    list2.addMouseClick(Qt::LeftButton);
    list2.addMouseClick(Qt::LeftButton);
    QTest::newRow("with delay") << list2 << 4;
}

void AddressBookLinkWidget::testButtons()
{
    QFETCH(QTestEventList, events);
    QFETCH(int, expected);
    QWidget *A = new QWidget(nullptr);
    Ui::AddressBookLinkWidgetBase ui;
    ui.setupUi(A);
    
    // check in UI if all three buttons Previous/Next/Close exists
    QSignalSpy spyClearButton(ui.btnClear, &QPushButton::clicked);
    
    events.simulate(ui.btnClear);
    QVERIFY(spyClearButton.isValid());
    QCOMPARE(spyClearButton.count(), expected);

    QSignalSpy spySelect(ui.btnSelectAddressee, &QPushButton::clicked);
    events.simulate(ui.btnSelectAddressee);
    QVERIFY(spySelect.isValid());
    QCOMPARE(spySelect.count(), expected);
}

void AddressBookLinkWidget::testLabel_data()
{
    QTest::addColumn<QString>("Test");
    QTest::newRow("char") << QStringLiteral("a");
    QTest::newRow("empty") << QString();
    QTest::newRow("special character") << QStringLiteral("*/**?\\*");
}

void AddressBookLinkWidget::testLabel()
{
    QFETCH(QString, Test);
    QWidget *A = new QWidget(nullptr);
    Ui::AddressBookLinkWidgetBase ui;
    ui.setupUi(A);
    ui.edtAddressee->setText(Test);
    QCOMPARE(ui.edtAddressee->text(), Test);
}

QTEST_MAIN(AddressBookLinkWidget)
#include "kopeteaddressbooklinkwidgettest.moc"
