/*
   kcodecaction.h

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyright (c) 2003-2006 by Michel Hermier         <michel.hermier@wanadoo.fr>
    Kopete    (c) 2003-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <kselectaction.h>

class KCodecAction
	: public KSelectAction
{
	Q_OBJECT

	Q_PROPERTY(QString codecName READ currentCodecName WRITE setCurrentCodec)
	Q_PROPERTY(int codecMib READ currentCodecMib)

public:
	KCodecAction(KActionCollection *parent, const QString &name);

	KCodecAction(const QString &text,
		KActionCollection *parent, const QString &name);

	KCodecAction(const KIcon &icon, const QString &text,
		KActionCollection *parent, const QString &name);

	virtual ~KCodecAction();

public:
	QTextCodec *currentCodec() const;
	bool setCurrentCodec(QTextCodec *codec);

	QString currentCodecName() const;
	bool setCurrentCodec(const QString &codecName);

	int currentCodecMib() const;
	bool setCurrentCodec(int mib);

signals:
	void triggered(QTextCodec *codec);

protected slots:
	virtual void actionTriggered(QAction *action);

private:
	void init();

	class Private;
	Private* const d;
};

#endif
