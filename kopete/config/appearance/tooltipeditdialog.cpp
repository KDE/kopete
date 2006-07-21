/*
    tooltipeditdialog.cpp  -  Kopete Tooltip Editor

    Copyright (c) 2004 by Stefan Gehn <metz AT gehn.net>
    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "tooltipeditdialog.h"

#include "kopetecontactproperty.h"
#include "kopeteglobal.h"
#include "kopeteappearancesettings.h"

#include <qapplication.h>
#include <qtoolbutton.h>
#include <qstringlist.h>
#include <q3header.h>

#include <kiconloader.h>
#include <k3listview.h>
#include <klocale.h>

class TooltipItem : public K3ListViewItem
{
	public:
		TooltipItem(K3ListView *parent, const QString& label, const QString& propertyName)
			: K3ListViewItem(parent, label),
				mPropName(propertyName)
		{
		}

		TooltipItem(K3ListView *parent, Q3ListViewItem *item, const QString& label, const QString& propertyName)
			: K3ListViewItem(parent, item, label),
				mPropName(propertyName)
		{
		}

		QString propertyName() const { return mPropName; }
	private:
		QString mPropName;
};



TooltipEditDialog::TooltipEditDialog(QWidget *parent)
	: KDialog(parent)
{
	setCaption( i18n("Tooltip Editor") );
	setButtons( KDialog::Ok | KDialog::Cancel );
	setDefaultButton(KDialog::Ok);
	showButtonSeparator(true);

	mMainWidget = new QWidget(this);
	mMainWidget->setObjectName("TooltipEditDialog::mMainWidget");
	setupUi(mMainWidget);

	setMainWidget(mMainWidget);
	lstUsedItems->header()->hide();
	lstUnusedItems->header()->hide();
	lstUsedItems->setSorting( -1 );
	lstUnusedItems->setSorting( 0 );

	const Kopete::ContactPropertyTmpl::Map propmap(
		Kopete::Global::Properties::self()->templateMap());
	QStringList usedKeys = Kopete::AppearanceSettings::self()->toolTipContents();

	connect(lstUnusedItems, SIGNAL(doubleClicked ( Q3ListViewItem *, const QPoint &, int )), this, SLOT(slotAddButton()));
	connect(lstUsedItems, SIGNAL(doubleClicked ( Q3ListViewItem *, const QPoint &, int )), this, SLOT(slotRemoveButton()));

	// first fill the "used" list
	foreach(QString usedProp, usedKeys)
	{
		if(propmap.contains(usedProp) && !propmap[usedProp].isPrivate())
		{
			new TooltipItem(lstUsedItems, propmap[usedProp].label(), usedProp);
		}
	}

	// then iterate over all known properties and insert the remaining ones
	// into the "unused" list
	Kopete::ContactPropertyTmpl::Map::ConstIterator it;
	for(it = propmap.begin(); it != propmap.end(); ++it)
	{
		if((usedKeys.contains(it.key())==0) && (!it.value().isPrivate()))
			new TooltipItem(lstUnusedItems, it.value().label(), it.key());
	}

	connect(lstUnusedItems, SIGNAL(selectionChanged(Q3ListViewItem *)),
		this, SLOT(slotUnusedSelected(Q3ListViewItem *)));
	connect(lstUsedItems, SIGNAL(selectionChanged(Q3ListViewItem *)),
		this, SLOT(slotUsedSelected(Q3ListViewItem *)));

	QIcon iconSet;
	iconSet = SmallIconSet("up");
	tbUp->setIcon(iconSet);
	tbUp->setEnabled(false);
	tbUp->setAutoRepeat(true);
	connect(tbUp, SIGNAL(clicked()), SLOT(slotUpButton()));

	iconSet = SmallIconSet("down");
	tbDown->setIcon(iconSet);
	tbDown->setEnabled(false);
	tbDown->setAutoRepeat(true);
	connect(tbDown, SIGNAL(clicked()), SLOT(slotDownButton()));

	iconSet = QApplication::isRightToLeft() ? SmallIconSet("back") : SmallIconSet("forward");
	tbAdd->setIcon(iconSet);
	tbAdd->setEnabled(false);
	connect(tbAdd, SIGNAL(clicked()), SLOT(slotAddButton()));

	iconSet = QApplication::isRightToLeft() ? SmallIconSet("forward") : SmallIconSet("back");
	tbRemove->setIcon(iconSet);
	tbRemove->setEnabled(false);
	connect(tbRemove, SIGNAL(clicked()), SLOT(slotRemoveButton()));

	connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));

	resize(QSize(450, 450));
}

void TooltipEditDialog::slotOkClicked()
{
	QStringList oldList = Kopete::AppearanceSettings::self()->toolTipContents();
	QStringList newList;
	Q3ListViewItemIterator it(lstUsedItems);
	QString keyname;

	while(it.current())
	{
		keyname = static_cast<TooltipItem *>(it.current())->propertyName();
		newList += keyname;
		kDebug(14000) << k_funcinfo <<
			"Adding key '" << keyname << "' to tooltip list" << endl;
		++it;
	}

	if(oldList != newList)
	{
		Kopete::AppearanceSettings::self()->setToolTipContents(newList);
		emit changed(true);
		kDebug(14000) << k_funcinfo << "tooltip fields changed, emitting changed()" << endl;
	}
}


void TooltipEditDialog::slotUnusedSelected(Q3ListViewItem *item)
{
	//tbRemove->setEnabled(false);
	tbAdd->setEnabled(item!=0);
}

void TooltipEditDialog::slotUsedSelected(Q3ListViewItem *item)
{
	tbRemove->setEnabled(item!=0);
	//tbAdd->setEnabled(false);
	if (item)
	{
		tbUp->setEnabled(item->itemAbove() != 0);
		tbDown->setEnabled(item->itemBelow() != 0);
	}
	else
	{
		tbUp->setEnabled(false);
		tbDown->setEnabled(false);
	}
}

void TooltipEditDialog::slotUpButton()
{
	Q3ListViewItem *item = lstUsedItems->currentItem();
	Q3ListViewItem *prev = item->itemAbove();
	if(prev == 0) // we are first item already
		return;

	prev->moveItem(item);
	slotUsedSelected(item);
}

void TooltipEditDialog::slotDownButton()
{
	Q3ListViewItem *item = lstUsedItems->currentItem();
	Q3ListViewItem *next = item->itemBelow();
	if(next == 0) // we are last item already
		return;

	item->moveItem(next);
	slotUsedSelected(item);
}

void TooltipEditDialog::slotAddButton()
{
	TooltipItem *item = static_cast<TooltipItem *>(lstUnusedItems->currentItem());
	if(!item)
		return;
	//kDebug(14000) << k_funcinfo << endl;

	// build a new one in the "used" list
	new TooltipItem(lstUsedItems, item->text(0), item->propertyName());

	// remove the old one from "unused" list
	lstUnusedItems->takeItem(item);
	delete item;
}

void TooltipEditDialog::slotRemoveButton()
{
	TooltipItem *item = static_cast<TooltipItem *>(lstUsedItems->currentItem());
	if(!item)
		return;
	//kDebug(14000) << k_funcinfo << endl;

	// build a new one in the "unused" list
	new TooltipItem(lstUnusedItems, item->text(0), item->propertyName());

	// remove the old one from "used" list
	lstUsedItems->takeItem(item);
	delete item;
}

#include "tooltipeditdialog.moc"
