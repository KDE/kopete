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

#include <QPushButton>
#include <KComboBox>
#include <QIcon>
#include <QSlider>
#include <QCheckBox>

class IdPushButton : public QPushButton
{
    Q_OBJECT
public:
    IdPushButton(unsigned int id, QWidget *parent = nullptr);
    IdPushButton(unsigned int id, const QString &text, QWidget *parent = nullptr);
    IdPushButton(unsigned int id, const QIcon &icon, const QString &text, QWidget *parent = nullptr);
private:
    unsigned int _id;
private Q_SLOTS:
    void emitPressed();
Q_SIGNALS:
    void pressed(unsigned int id);
};

class IdComboBox : public KComboBox
{
    Q_OBJECT
public:
    IdComboBox(unsigned int id, QWidget *parent = nullptr);
    IdComboBox(unsigned int id, bool rw, QWidget *parent = nullptr);
private:
    unsigned int _id;
private Q_SLOTS:
    void emitCurrentIndexChanged(int index);
Q_SIGNALS:
    void currentIndexChanged(unsigned int id, int index);
};

class IdCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    IdCheckBox(unsigned int id, QWidget *parent = nullptr);
    IdCheckBox(unsigned int id, const QString &text, QWidget *parent = nullptr);
private:
    unsigned int _id;
private Q_SLOTS:
    void emitStateChanged(int state);
Q_SIGNALS:
    void stateChanged(unsigned int id, int state);
};

class IdSlider : public QSlider
{
    Q_OBJECT
public:
    IdSlider(unsigned int id, QWidget *parent = nullptr);
    IdSlider(unsigned int id, Qt::Orientation orientation, QWidget *parent = nullptr);
private:
    unsigned int _id;
private Q_SLOTS:
    void emitValueChanged(int value);
Q_SIGNALS:
    void valueChanged(unsigned int id, int value);
};

#endif
