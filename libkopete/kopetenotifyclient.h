/* This file is part of the KDE libraries
   Copyright (C) 2000 Charles Samuels <charles@kde.org>

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

#ifndef _KOPETENOTIFY_CLIENT_
#define _KOPETENOTIFY_CLIENT_


#include <kguiitem.h>
#include <knotifyclient.h>

class QObject;
class KopeteMetaContact;
class KopeteNotifyDataObject;

namespace KNotifyClient
{
	/**
	 * This should be the most used method in here.
	 * Pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 *
	 * Call it by KNotifyClient::event(widget->winId(), "EventName");
	 * It will use KApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @param winId The winId() of the widget where the event originates
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be QString::null.
	 * @param action The text to show in the message box, or in the passive popup
	 * @param receiver The @p slot's parent
	 * @param slot The SLOT to invoque when the @p action is fired
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.2
	 */
int event(int winId, const QString &message, const QString &text, const KGuiItem &action,
                    QObject* receiver , const char* slot);

	/**
	 * This should be the most used method in here.
	 * Pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 *
	 * Call it by KNotifyClient::event(widget->winId(), "EventName");
	 * It will use KApplication::kApplication->dcopClient() to communicate to
	 * the server
	 * @param winId The winId() of the widget where the event originates
	 * @param message The name of the event
	 * @param text The text to put in a dialog box.  This won't be shown if
	 *             the user connected the event to sound, only. Can be QString::null.
	 * @param mc The metacontact to which the notification relates.  Used for custom notifications, may not be 0.
	 * @param action The text to show in the message box, or in the passive popup
	 * @param receiver The @p slot's parent
	 * @param slot The SLOT to invoque when the @p action is fired
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.2
	 */
int event( int winId, const QString& message, const QString& text, KopeteMetaContact *mc,
		const KGuiItem& action, QObject *receiver, const char *slot );

	/**
	 * Will fire an event that's not registered.
	 * You should
	 * pass the origin-widget's winId() here so that a PassivePopup can be
	 * placed appropriately.
	 * @param winId The winId() of the originating widget
	 * @param text The error message text, if applicable
	 * @param present The presentation method(s) of the event
	 * @param level The error message level, defaulting to "Default"
	 * @param sound The sound file to play if selected with @p present
	 * @param file The log file to play if selected with @p present
	 * @param commandline The command to execute if selected with @p present
	 * @param action The text to show in the message box, or in the passive popup
	 * @param receiver The @p slot's parent
	 * @param slot The SLOT to invoque when the @p action is fired
	 * @return a value > 0, unique for this event if successful, 0 otherwise
	 * @since 3.2
	 */
	int userEvent(int winId, const QString &message, const QString &text, int present, int level, 
			 const QString &sound, const QString &file, const QString &commandline,
			 const KGuiItem &action = KGuiItem() , QObject *receiver=0L, const char *slot=0L);

	void performCustomNotifications( int winId, KopeteNotifyDataObject *dataObj, const QString &message, bool& suppress);
}

#endif

// vim: set noet ts=4 sts=4 tw=4:

