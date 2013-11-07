/*
    IdGuiElements.h - Modified GUI-elements which send a selectable ID with their signals

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
#ifndef IDGUIELEMENTS_H
#define IDGUIELEMENTS_H


#include <KPushButton>
#include <KComboBox>
#include <QSlider>
#include <QCheckBox>


class IdPushButton : public KPushButton
{
	Q_OBJECT
public:
	IdPushButton(unsigned int id, QWidget * parent = 0);
	IdPushButton(unsigned int id, const QString text, QWidget * parent = 0);
	IdPushButton(unsigned int id, const KIcon icon, const QString text, QWidget * parent = 0);
	IdPushButton(unsigned int id, KGuiItem item, QWidget * parent = 0);
private:
	unsigned int _id;
private slots:
	void emitPressed();
signals:
	void pressed(unsigned int id);
};


class IdComboBox : public KComboBox
{
	Q_OBJECT
public:
	IdComboBox(unsigned int id, QWidget * parent = 0);
	IdComboBox(unsigned int id, bool rw, QWidget * parent = 0);
private:
	unsigned int _id;
private slots:
	void emitCurrentIndexChanged(int index);
signals:
	void currentIndexChanged(unsigned int id, int index);
};


class IdCheckBox : public QCheckBox
{
	Q_OBJECT
public:
	IdCheckBox(unsigned int id, QWidget * parent = 0);
	IdCheckBox(unsigned int id, const QString & text, QWidget * parent = 0);
private:
	unsigned int _id;
private slots:
	void emitStateChanged(int state);
signals:
	void stateChanged(unsigned int id, int state);
};


class IdSlider : public QSlider
{
	Q_OBJECT
public:
	IdSlider(unsigned int id, QWidget * parent = 0);
	IdSlider(unsigned int id, Qt::Orientation orientation, QWidget * parent = 0);
private:
	unsigned int _id;
private slots:
	void emitValueChanged(int value);
signals:
	void valueChanged(unsigned int id, int value);
};


#endif

