/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SMSPREFERENCES_H
#define SMSPREFERENCES_H

#include "configmodule.h"
#include "kopeteprefs.h"
#include "smsprefs.h"

class QBoxLayout;
class SMSService;

class SMSPreferences : public ConfigModule
{
	Q_OBJECT

public:
	SMSPreferences( const QString &pixmap, QObject *parent = 0 );
	~SMSPreferences();
	virtual void save();

signals:
	void saved();

private:
	smsPrefsUI *preferencesDialog;
	QGroupBox *configVBox;
	QBoxLayout *configLayout;
	QWidget *configWidget;
	SMSService *service;
public slots: // Public slots
  /** No descriptions */
  virtual void reopen();
  void setServicePreferences(const QString& name);
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

