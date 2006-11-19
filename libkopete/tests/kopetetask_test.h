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
#ifndef KOPETETASK_TEST_H
#define KOPETETASK_TEST_H

#include <QObject>

#include <kopetetask.h>

class ParseEmoticonTask : public Kopete::Task
{
	Q_OBJECT
public:
	ParseEmoticonTask(const QString &sourceString);

	QString parseResult() const { return m_parsed; }

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

#endif
