/*
    kopeteawayconfigui.cpp  -  Kopete Away Config UI

    Copyright (c) 2002      by Chris TenHarmsel       <ctenha56@calvin.edu>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteawayconfigui.h"
#include "awaymessageeditor.h"

#include <qtextedit.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

#include "kopeteaway.h"

KopeteAwayConfigUI::KopeteAwayConfigUI(QWidget *parent) :
	KopeteAwayConfigBaseUI(parent)
{
	connect(btnAddAwayMsg, SIGNAL(clicked()),
		this, SLOT(btnAddAwayMsgClicked()));
	connect(btnDeleteAwayMsg, SIGNAL(clicked()),
		this, SLOT(btnDeleteAwayMsgClicked()));
	connect(btnEditAwayMsg, SIGNAL(clicked()),
		 this, SLOT(btnEditAwayMsgClicked()));
	connect(lstTitles, SIGNAL(selectionChanged()), this, SLOT(titleSelected()));
}

void KopeteAwayConfigUI::btnAddAwayMsgClicked()
{
	KDialogBase* editor = new KDialogBase(this, "AwayMessageEditor", true,
		i18n("Add Away Message"),
		KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok );

	AwayMessageEditor *editorWidget = new AwayMessageEditor(editor,
		"AwayMessageEditorWidget");
	editor->setMainWidget(editorWidget);

	if (editor->exec() == QDialog::Accepted)
	{
		KopeteAway::getInstance()->addMessage(editorWidget->txtMessage->text(),
			editorWidget->txtMessageText->text());
		updateView();
		emit awayMessagesChanged(true);
	}

	delete editor;
}

void KopeteAwayConfigUI::btnDeleteAwayMsgClicked()
{
	if (lstTitles->currentItem() == -1) return;

	QListBoxItem *selectedItem = lstTitles->item(lstTitles->currentItem());
	int retval = KMessageBox::warningYesNo(this,
		i18n("Delete message '%1'?").arg(selectedItem->text()),
		i18n("Delete Message"));

	if (retval == KMessageBox::Yes)
	{
		KopeteAway::getInstance()->deleteMessage(selectedItem->text());
		updateView();
		emit awayMessagesChanged(true);
	}
}

void KopeteAwayConfigUI::btnEditAwayMsgClicked()
{
	if (lstTitles->currentItem() == -1)
		return;

	QListBoxItem *selectedItem = lstTitles->item(lstTitles->currentItem());

	KDialogBase* editor = new KDialogBase(0, "AwayMessageEditor", true,
		i18n("Edit Away Message"),
		KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok );

	AwayMessageEditor *editorWidget = new AwayMessageEditor(editor,
		"AwayMessageEditorWidget");
	editor->setMainWidget(editorWidget);
	editorWidget->txtMessage->setText(selectedItem->text());
	editorWidget->txtMessageText->setText(txtMessage->text());

	if (editor->exec() == QDialog::Accepted)
	{
		KopeteAway::getInstance()->updateMessage(editorWidget->txtMessage->text(),
			editorWidget->txtMessageText->text());
		updateView();
		emit awayMessagesChanged(true);
	}

	delete editor;
}

void KopeteAwayConfigUI::titleSelected()
{
	QListBoxItem *selectedItem = lstTitles->item(lstTitles->currentItem());
	txtMessage->setText(
		KopeteAway::getInstance()->getMessage(selectedItem->text()));
}

void KopeteAwayConfigUI::updateView()
{
	lstTitles->clear();
	QStringList titles = KopeteAway::getInstance()->getTitles();

	for (QStringList::iterator i = titles.begin(); i != titles.end(); i++)
		lstTitles->insertItem(*i); // Insert the Title into the list

	txtMessage->setText(QString::null);
}

#include "kopeteawayconfigui.moc"
