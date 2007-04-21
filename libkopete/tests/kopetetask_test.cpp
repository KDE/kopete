/*
    Tests for Kopete::Task

    Copyright (c) 2006      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qtest_kde.h>

#include <QtTest/QSignalSpy>
#include <QTimer>

#include "kopetetask_test.h"
#include "kopetetask_test.moc"

#include <kcmdlineargs.h>
#include <kdebug.h>

#include <kopetetask.h>
#include <kopeteemoticons.h>

QTEST_KDEMAIN( KopeteTaskTest, GUI )

const QString sampleString = QString("Sample string :) :D ;)");

ParseEmoticonTask::ParseEmoticonTask(const QString &sourceString)
 : Kopete::Task(), m_source(sourceString)
{
	QTimer::singleShot(0, this, SLOT(start()));
}

void ParseEmoticonTask::start()
{
	parseEmoticon(m_source);
}

void ParseEmoticonTask::parseEmoticon(const QString &value)
{
	Kopete::Emoticons emo("Default");
	m_parsed = emo.parse(value, Kopete::Emoticons::RelaxedParse | Kopete::Emoticons::SkipHTML);

	Kopete::Task::Result jobResult = Kopete::Task::ResultSucceeded;
	if(m_parsed.isEmpty())
		jobResult = Kopete::Task::ResultFailed;

	emitResult();
}

void KopeteTaskTest::testEmoticonTask()
{
	ParseEmoticonTask *task = new ParseEmoticonTask(sampleString);
	QSignalSpy spy(task, SIGNAL(result(KJob*)));

	// For the task to execute, we must manually call the event loop.
	qApp->processEvents();
	
	QCOMPARE(spy.count(), 1);
	QCOMPARE(task->succeeded(), true);
}
