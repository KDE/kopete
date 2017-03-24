/*
    avatardialog.cpp - Dialog to manage and select user avatar

    Copyright (c) 2007      by Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
    Copyright (c) 2007      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "avatardialog.h"

// Qt includes
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

// KDE includes
#include <KConfigGroup>
#include <KLocalizedString>

// Kopete includes
#include "avatarselectorwidget.h"
#include "kopeteavatarmanager.h"

namespace Kopete
{

namespace UI
{

class AvatarDialog::Private
{
public:
    Private()
        : mainWidget(0)
    {}

    AvatarSelectorWidget *mainWidget;
    QString selectedPath;
    QString currentPath;
};

AvatarDialog::AvatarDialog(QWidget *parent)
    : QDialog(parent), d(new Private)
{
    setWindowTitle( i18n("Select Avatar") );
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    d->mainWidget = new Kopete::UI::AvatarSelectorWidget(this);
    mainLayout->addWidget(d->mainWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AvatarDialog::slotOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

AvatarDialog::~AvatarDialog()
{
    delete d;
}

QString AvatarDialog::selectedAvatarPath() const
{
    return d->selectedPath;
}

QString AvatarDialog::getAvatar(QWidget *parent, const QString &currentAvatar, bool * ok )
{
    QPointer <AvatarDialog> dialog = new AvatarDialog(parent);
    dialog->d->mainWidget->setCurrentAvatar(currentAvatar);
    dialog->d->currentPath = currentAvatar;
    if ( dialog->exec() == QDialog::Accepted )
    {
        if ( ok ) {
            *ok = true;
        }
    }
    else
    {
        if ( ok ) {
            *ok = false;
        }
    }
    QString ret;
    if ( dialog )
        ret = dialog->selectedAvatarPath();
    delete dialog;
    return ret;
}

void AvatarDialog::slotOkClicked()
{
    Kopete::AvatarManager::AvatarEntry selectedEntry = d->mainWidget->selectedEntry();

    d->selectedPath = selectedEntry.path;
    emit result();
    accept();
}


} // namespace UI

} // namespace Kopete
