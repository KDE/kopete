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

using namespace QtTapioca;

/**
 * @brief Small label and line edit for a single ConnectionManager::Parameter
 */
class ParameterLineEdit : public QWidget
{
	// TODO Support flags
public:
	ParameterLineEdit(const ConnectionManager::Parameter &parameter, QWidget *parent)
	 : QWidget(parent), m_lineValue(0)
	{
		m_parameter = parameter;
		createWidget();
	}

	ConnectionManager::Parameter parameter() const
	{
		return m_parameter;
	}

	void createWidget()
	{
		QVBoxLayout *mainLayout = new QVBoxLayout(this);
		
		// Create name label
		mainLayout->addWidget( new QLabel(parameter().name(), this) );

		// Create editable value field
		m_lineValue = new QLineEdit(this);
		// Set a value if any
		m_lineValue->setText( parameter().value().toString() );
		mainLayout->addWidget(m_lineValue);

		// Set a spacing item
		mainLayout->addSpacing(10);
	}

	QString name() const
	{
		return m_parameter.name();
	}

	QString value() const
	{
		QString temp = m_lineValue->text();
		return temp;
	}

private:
	ConnectionManager::Parameter m_parameter;
	QLineEdit *m_lineValue;
};

class TelepathyEditParameterWidget::Private
{
public:
	Private()
	 : mainLayout(0)
	{}

	void init(QWidget *parent);
	void createWidgets(QWidget *parent);
	void clear();

	QGridLayout *mainLayout;
	QList<ConnectionManager::Parameter> paramList;
	QList<ParameterLineEdit*> lineEditList;
};

TelepathyEditParameterWidget::TelepathyEditParameterWidget(const QList<ConnectionManager::Parameter> &paramList, QWidget *parent)
 : QWidget(parent), d(new Private)
{
	d->paramList = paramList;
	d->init(this);
}

TelepathyEditParameterWidget::~TelepathyEditParameterWidget()
{
	kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << endl;
	delete d;
}

QList<QtTapioca::ConnectionManager::Parameter> TelepathyEditParameterWidget::parameterList()
{
	QList<ConnectionManager::Parameter> parameterList;

	foreach(ParameterLineEdit *lineEdit, d->lineEditList)
	{
		if( !lineEdit )
		{
			kDebug(TELEPATHY_DEBUG_AREA) << k_funcinfo << "WARNING: A ParameterLineEdit is null !" << endl;
		}
		ConnectionManager::Parameter updatedParameter(lineEdit->name(), lineEdit->value());
		parameterList.append(updatedParameter);
	}

	return parameterList;
}

void TelepathyEditParameterWidget::setParameterList(const QList<QtTapioca::ConnectionManager::Parameter> &parameterList)
{
	d->clear();
	d->paramList = parameterList;
	d->createWidgets(this);
}

void TelepathyEditParameterWidget::Private::init(QWidget *parent)
{
	mainLayout = new QGridLayout(parent);

	createWidgets(parent);

// 	mainLayout->addStretch(2);
}

void TelepathyEditParameterWidget::Private::createWidgets(QWidget *parent)
{
	int column=0, row=0;
	foreach(ConnectionManager::Parameter parameter, paramList)
	{
		ParameterLineEdit *lineEdit = new ParameterLineEdit(parameter, parent);
		mainLayout->addWidget(lineEdit, row, column);
		lineEditList.append(lineEdit);
	
		if( ++row >= 5 )
		{
			column++;
			row = 0;
		}
	}
}

void TelepathyEditParameterWidget::Private::clear()
{
	qDeleteAll(lineEditList);
}

#include "telepathyeditparameterwidget.moc"
