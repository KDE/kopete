/*
    Tests for Kopete::Task

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

#include <QObject>
#include <QtTest>
#include <QSignalSpy>
#include <QTimer>

#include <kopetetask.h>
#include <kopeteemoticons.h>


class ParseEmoticonTask : public Kopete::Task
{
    Q_OBJECT

public:
    ParseEmoticonTask(const QString &sourceString);

    QString parseResult() const
    {
        return m_parsed;
    }

private slots:
    void start();

private:
    void parseEmoticon(const QString &value);

    QString m_source;
    QString m_parsed;
};

class KopeteTaskTest : public QObject
{
    Q_OBJECT
private slots:
    void testEmoticonTask();
};


const QString sampleString = QString("Sample string :) :D ;)");

ParseEmoticonTask::ParseEmoticonTask(const QString &sourceString)
    : Kopete::Task(), m_source(sourceString)
{
    QTimer::singleShot(0, this, &ParseEmoticonTask::start);
}

void ParseEmoticonTask::start()
{
    parseEmoticon(m_source);
}

void ParseEmoticonTask::parseEmoticon(const QString &value)
{
    m_parsed = Kopete::Emoticons::parseEmoticons(value, KEmoticonsTheme::RelaxedParse | KEmoticonsTheme::SkipHTML);

    if (m_parsed.isEmpty()) {
        setError(100);
    }

    emitResult();
}

void KopeteTaskTest::testEmoticonTask()
{
    ParseEmoticonTask *task = new ParseEmoticonTask(sampleString);
    QSignalSpy spy(task, SIGNAL(result(KJob*)));

    // For the task to execute, we must manually call the event loop.
    qApp->processEvents();

    QCOMPARE(spy.count(), 1);
    QCOMPARE(task->error(), 0);
}

QTEST_MAIN(KopeteTaskTest)
#include "kopetetasktest.moc"
