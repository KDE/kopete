/*
    IdGuiElements.cpp - Modified GUI-elements which send a selectable ID with their signals

    Copyright (c) 2010 by Frank Schaefer        <fschaefer.oss@googlemail.com>

    Kopete    (c) 2010 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
 */
#include "IdGuiElements.h"



IdPushButton::IdPushButton(unsigned int id, QWidget * parent) : KPushButton(parent)
{
	_id = id;
	connect(this, SIGNAL(pressed()), this, SLOT(emitPressed()));
}

IdPushButton::IdPushButton(unsigned int id, const QString text, QWidget * parent) : KPushButton(text, parent)
{
	_id = id;
	connect(this, SIGNAL(pressed()), this, SLOT(emitPressed()));
}

IdPushButton::IdPushButton(unsigned int id, const KIcon icon, const QString text, QWidget * parent) : KPushButton(icon, text, parent)
{
	_id = id;
	connect(this, SIGNAL(pressed()), this, SLOT(emitPressed()));
}

IdPushButton::IdPushButton(unsigned int id, KGuiItem item, QWidget * parent) : KPushButton(item, parent)
{
	_id = id;
	connect(this, SIGNAL(pressed()), this, SLOT(emitPressed()));
}

void IdPushButton::emitPressed()
{
	emit pressed(_id);
}



IdComboBox::IdComboBox(unsigned int id, QWidget * parent) : KComboBox(parent)
{
	_id = id;
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(emitCurrentIndexChanged(int)));
}

IdComboBox::IdComboBox(unsigned int id, bool rw, QWidget * parent) : KComboBox(rw, parent)
{
	_id = id;
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(emitCurrentIndexChanged(int)));
}

void IdComboBox::emitCurrentIndexChanged(int index)
{
	emit currentIndexChanged(_id, index);
}



IdCheckBox::IdCheckBox(unsigned int id, QWidget * parent) : QCheckBox(parent)
{
	_id = id;
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(emitStateChanged(int)));
}

IdCheckBox::IdCheckBox(unsigned int id, const QString & text, QWidget * parent) : QCheckBox(text, parent)
{
	_id = id;
	connect(this, SIGNAL(stateChanged(int)), this, SLOT(emitStateChanged(int)));
}

void IdCheckBox::emitStateChanged(int state)
{
	if (state > 1) state = 1;	// NOTE: Problem: state = Qt::Unchecked = 0 or Qt::Checked = 2;
	emit stateChanged(_id, state);
}



IdSlider::IdSlider(unsigned int id, QWidget * parent) : QSlider(parent)
{
	_id = id;
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(emitValueChanged(int)));

}

IdSlider::IdSlider(unsigned int id, Qt::Orientation orientation, QWidget * parent) : QSlider(orientation, parent)
{
	_id = id;
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(emitValueChanged(int)));
}

void IdSlider::emitValueChanged(int value)
{
	emit valueChanged(_id, value);
}

