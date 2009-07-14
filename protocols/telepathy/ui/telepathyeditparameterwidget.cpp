/*
 * telepathyeditparameterwidget.cpp - UI to edit Telepathy connection parameter
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
#include "telepathyeditparameterwidget.h"

// Qt includes
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

// KDE includes
#include <kdebug.h>

// Local includes
#include "telepathyprotocol.h"

/**
 * @brief Small label and line edit for a single Tp::ProtocolParameterList
 */
class ParameterLineEdit : public QWidget
{
    // TODO Support flags
public:
    ParameterLineEdit(Tp::ProtocolParameter *parameter, QWidget *parent)
            : QWidget(parent), m_lineValue(0) {
        kDebug(TELEPATHY_DEBUG_AREA) << parameter->name() << " " << parameter->dbusSignature().signature() << " " << parameter->defaultValue().toString();
        m_parameter = parameter;
        createWidget();
    }

    Tp::ProtocolParameter* parameter() const {
        return m_parameter;
    }

    void createWidget() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Create name label
        mainLayout->addWidget(new QLabel(parameter()->name(), this));

        // Create editable value field
        m_lineValue = new QLineEdit(this);
        // Set a value if any
        m_lineValue->setText(parameter()->defaultValue().toString());
        mainLayout->addWidget(m_lineValue);

        // Set a spacing item
        mainLayout->addSpacing(10);
    }

    QString name() const {
        return m_parameter->name();
    }

    QString value() const {
        QString temp = m_lineValue->text();
        return temp;
    }

private:
    Tp::ProtocolParameter *m_parameter;
    QLineEdit *m_lineValue;
};

class TelepathyEditParameterWidget::Private
{
public:
    Private()
            : mainLayout(0) {}

    void init(QWidget *parent);
    void createWidgets(QWidget *parent);
    void clear();

    QGridLayout *mainLayout;
    Tp::ProtocolParameterList paramList;
    QList<ParameterLineEdit*> lineEditList;
};

TelepathyEditParameterWidget::TelepathyEditParameterWidget(const Tp::ProtocolParameterList &paramList, QWidget *parent)
        : QWidget(parent), d(new Private)
{
    d->paramList = paramList;
    d->init(this);
}

TelepathyEditParameterWidget::~TelepathyEditParameterWidget()
{
    kDebug(TELEPATHY_DEBUG_AREA) ;
    delete d;
}

Tp::ProtocolParameterList TelepathyEditParameterWidget::parameterList()
{
    Tp::ProtocolParameterList parameterList;

    foreach(ParameterLineEdit *lineEdit, d->lineEditList) {
        if (!lineEdit) {
            kDebug(TELEPATHY_DEBUG_AREA) << "WARNING: A ParameterLineEdit is null !";
        }
        Tp::ProtocolParameter *updatedParameter;
        updatedParameter = new Tp::ProtocolParameter(
            lineEdit->name(),
            lineEdit->parameter()->dbusSignature(),
            lineEdit->value(),
            Tp::ConnMgrParamFlagHasDefault
        );
        parameterList.append(updatedParameter);
    }

    return parameterList;
}

void TelepathyEditParameterWidget::setParameterList(const Tp::ProtocolParameterList &parameterList)
{
    d->clear();
    d->paramList = parameterList;
    d->createWidgets(this);
}

void TelepathyEditParameterWidget::Private::init(QWidget *parent)
{
    mainLayout = new QGridLayout(parent);

    createWidgets(parent);
}

void TelepathyEditParameterWidget::Private::createWidgets(QWidget *parent)
{
    int column = 0, row = 0;
    foreach(Tp::ProtocolParameter *parameter, paramList) {
        ParameterLineEdit *lineEdit = new ParameterLineEdit(parameter, parent);
        mainLayout->addWidget(lineEdit, row, column);
        lineEditList.append(lineEdit);

        if (++row >= 5) {
            column++;
            row = 0;
        }
    }
}

void TelepathyEditParameterWidget::Private::clear()
{
    qDeleteAll(lineEditList);
    qDeleteAll(paramList);
}

#include "telepathyeditparameterwidget.moc"
