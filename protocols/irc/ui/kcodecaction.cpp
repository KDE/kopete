/*
    kcodecaction.cpp

    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyrigth (c) 2006      by Michel Hermier         <michel.hermier@wanadoo.fr>
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

#include "kcodecaction.h"

#include <kcharsets.h>
#include <kdebug.h>
#include <klocale.h>

#include <qtextcodec.h>

// Acording to http://www.iana.org/assignments/ianacharset-mib
// the default/unknown mib value is 2. 
#define DEFAULT_MIB 2

class KCodecAction::Private
{
public:
    Private()
        : defaultAction(0)
        , configureAction(0)
        , currentCodecMib(DEFAULT_MIB)
    {
    }

    QAction *defaultAction;
    QAction *configureAction;

    int currentCodecMib;
};

KCodecAction::KCodecAction( KActionCollection *parent, const QString &name )
    : KSelectAction( parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::KCodecAction( const KIcon &icon, const QString &text, KActionCollection *parent, const QString &name )
    : KSelectAction( icon, text, parent, name )
    , d(new Private)
{
    init();
}

KCodecAction::~KCodecAction()
{
    delete d;
}

void KCodecAction::init()
{
    d->defaultAction = addAction(i18n("Default"));
    foreach(QString encodingName, KGlobal::charsets()->descriptiveEncodingNames())
       addAction(encodingName);
#if 0
    addSeparator();
    d->configureAction = addAction(i18n("Configure"));
    d->configureAction->setCheckable(false);
#endif
    setCurrentItem(0);

//    setEditable(true);
}

void KCodecAction::actionTriggered(QAction *action)
{
#if 0
    if (action == d->configureAction)
    {
        // Configure the menu content
        return;
    }
#endif

    // cache values so we don't need access to members in the action
    // after we've done an emit() (done in KSelectAction::actionTriggered)
    QTextCodec *codec = currentCodec();
    if (codec)
    {
        KSelectAction::actionTriggered(action);
        emit triggered(codec);
    }
#if 0
    else
    {
//        Warn the user in the gui somehow ?
    }
#endif
}

QTextCodec *KCodecAction::currentCodec() const
{
    QAction *action = currentAction();
    QTextCodec *codec = 0;

    if (action == d->defaultAction)
    {
        // Take default codec
        // FIXME offer to change the default codec
        codec = QTextCodec::codecForLocale();
    }
    else
    {
        bool ok = false;
        KCharsets *charsets = KGlobal::charsets();
        QString encoding = charsets->encodingForName(action->text());
        codec = charsets->codecForName(encoding, ok);
        if (!ok)
        {
            kWarning() << k_funcinfo << "Invalid codec convertion for :\""  << action->text() << '"' << endl;
            return 0;
        }
    }

    return codec;
}

bool KCodecAction::setCurrentCodec( QTextCodec *codec )
{
    if (codec)
        return setCurrentAction(codec->name());
    else
        return false;
}

QString KCodecAction::currentCodecName() const
{
    return currentText();
}

bool KCodecAction::setCurrentCodec( const QString &codecName )
{
    return setCurrentAction(codecName, Qt::CaseInsensitive);
}

int KCodecAction::currentCodecMib() const
{
    QTextCodec *codec = currentCodec();
    if (codec)
        return codec->mibEnum();
    else
        return DEFAULT_MIB;
}

bool KCodecAction::setCurrentCodec( int mib )
{
    if (mib == DEFAULT_MIB)
        return setCurrentAction(d->defaultAction);
    else
        return setCurrentCodec( QTextCodec::codecForMib(mib) );
}

#include "kcodecaction.moc"

