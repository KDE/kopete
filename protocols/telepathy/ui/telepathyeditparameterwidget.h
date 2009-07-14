/*
 * telepathyeditparameterwidget.h - UI to edit Telepathy connection parameter
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef TELEPATHYEDITPARAMETERWIDGET_H
#define TELEPATHYEDITPARAMETERWIDGET_H

#include <QtGui/QWidget>

#include <TelepathyQt4/ConnectionManager>

class TelepathyEditParameterWidget : public QWidget
{
    Q_OBJECT
public:
    TelepathyEditParameterWidget(const Tp::ProtocolParameterList &paramsList, QWidget *parent);
    ~TelepathyEditParameterWidget();

    /**
     * @brief Get the modified parameters as a list of Tp::ProtocolParameterList
     */
    Tp::ProtocolParameterList parameterList();

    /**
     * @brief Set the parameter read mostly from the config file.
     */
    void setParameterList(const Tp::ProtocolParameterList &parameterList);
private:
    class Private;
    Private *d;
};


#endif  // header guard

