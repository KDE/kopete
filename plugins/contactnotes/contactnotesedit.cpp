/*
    contactnotesedit.cpp  -  description

    Copyright (c) 2002      by Olivier Goffart <ogoffart@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    ***************************************************************************
    *                                                                         *
    *   This program is free software; you can redistribute it and/or modify  *
    *   it under the terms of the GNU General Public License as published by  *
    *   the Free Software Foundation; either version 2 of the License, or     *
    *   (at your option) any later version.                                   *
    *                                                                         *
    ***************************************************************************
*/

#include "contactnotesedit.h"

#include <qlabel.h>
#include <QTextEdit>

#include <KLocalizedString>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

#include "kopetemetacontact.h"

#include "contactnotesplugin.h"

ContactNotesEdit::ContactNotesEdit(Kopete::MetaContact *m, ContactNotesPlugin *p)
    : QDialog()
{
    setWindowTitle(i18n("Contact Notes"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ContactNotesEdit::slotOkButtonClicked);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    
    m_plugin = p;
    m_metaContact = m;

    QWidget *w = new QWidget(this);
    QVBoxLayout *wVBoxLayout = new QVBoxLayout(w);
    wVBoxLayout->setMargin(0);
    m_label = new QLabel(i18n("Notes about %1:", m->displayName()), w);
    wVBoxLayout->addWidget(m_label);
    m_label->setObjectName(QStringLiteral("m_label"));
    m_linesEdit = new QTextEdit(w);
    wVBoxLayout->addWidget(m_linesEdit);

    m_linesEdit->setText(p->notes(m));

    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
}

ContactNotesEdit::~ContactNotesEdit()
{
}

void ContactNotesEdit::slotOkButtonClicked()
{
    emit notesChanged(m_linesEdit->toPlainText(), m_metaContact);
}
