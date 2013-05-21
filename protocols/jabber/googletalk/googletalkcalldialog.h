/*
    googletalkcalldialog.h - Google Talk and Google libjingle support

    Copyright (c) 2009 by Pali Rohár <pali.rohar@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef GoogleTalkCallDialog_H
#define GoogleTalkCallDialog_H

#include "ui_googletalkcalldialog.h"

class QCloseEvent;

/**
 * @author Pali Rohár
 * @short Dialog for voice call
 * This is voice call dialog for Google libjingle example googletalk-call application
 */
class GoogleTalkCallDialog : public QDialog, public Ui::GoogleTalkCallDialog
{

	Q_OBJECT

	public:

		/**
		 * Constructor for voice call dialog
		 * use method show() to open and show voice call dialog
		 * use method hide() to close and hide voice call dialog
		 */
		GoogleTalkCallDialog(QWidget *parent = 0);

	protected:

		/**
		 * Reimplement close event
		 * Do not close and do not delete this call dialog, but emit signal closed()
		 */
		virtual void closeEvent(QCloseEvent * e);

	signals:

		void closed();

};

#endif // GoogleTalkCallDialog_H

