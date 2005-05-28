/*  This file is part of the KDE project
    Copyright (C) 2005 Michal Vaner <michal.vaner@kdemail.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef SKYPECALLDIALOG_H
#define SKYPECALLDIALOG_H

#include <skypecalldialogbase.h>

class SkypeAccount;
class SkypeCallDialogPrivate;

/**
 * This class is a window that can control a call (show information about it, hang up, hold, ...)
 * @author Michal Vaner (Vorner)
 */
class SkypeCallDialog : public SkypeCallDialogBase
{
	Q_OBJECT
	private:
		SkypeCallDialogPrivate *d;
		///Start timeout to close
		void closeLater();
	private slots:
		///Close it after timeout after finishing the call
		void deathTimeout();
		///Update the call info now
		void updateCallInfo();
	protected slots:
		///The accept button was clicked, accept the call
		virtual void acceptCall();
		///Hold or release the call
		virtual void holdCall();
		///Hang up the call
		virtual void hangUp();
		///I want to know when I'm closed
		virtual void closeEvent(QCloseEvent *e);
	public:
		/**
		 * Constructor
		 */
		SkypeCallDialog(const QString &callId, const QString &userId, SkypeAccount *account);
		///Destructor
		~SkypeCallDialog();
	public slots:
		///Update the status of call and disable/enable the right buttons and show it in the labels
		void updateStatus(const QString &callId, const QString &status);
		///Updates an error message when some error occured
		void updateError(const QString &callId, const QString &status);
		/**
		 * Incoming skype-out balance info
		 * @param balance How much of that ddoes user have
		 * @param currency What currency is it (actually only euro-cents are used)
		 */
		void skypeOutInfo(int balance, const QString &currency);
	signals:
		/**
		 * accept an incoming call
		 * @param callId What call is it
		 */
		void acceptTheCall(const QString &callId);
		/**
		 * Hang up this call for me, please
		 * @param callId What call are we talking about
		 */
		void hangTheCall(const QString &callId);
		/**
		 * Hold or resume a call (depending on its actual status
		 * @param callId What call are we tlking about
		 */
		void toggleHoldCall(const QString &callId);
		/**
		 * Tell me the skype out balance, please
		 */
		void updateSkypeOut();
};

#endif
