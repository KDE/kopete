/* This file is part of the KDE libraries
   Copyright (C) 2005 Olivier Goffart <ogoffart @ kde.org>

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


#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H


#include <qpixmap.h>
#include <qobject.h>
#include <qstringlist.h>
#include <kdemacros.h>
#include <qpair.h>

class QWidget;

/**
 * KNotification is used to notify some event to the user.
 *
 * It covers severals kind of notifications
 *
 * @li Interface feedback events:
 * For notifying the user that he/she just performed an operation, like maximizing a
 * window. This allows us to play sounds when a dialog appears.
 * This is an instant notification.  It ends automatically after a small timeout
 *
 * @li complex notifications:
 * Notify when one received a new message, or when something important happened
 * the user has to know.  This notification has a start and a end.  It start when
 * the event actually occurs, and finish when the message is acknowledged.
 *
 *
 * use the static function event() to fire an event
 *
 * the returned KNotification pointer may be used to connect signals or slots
 * 
 * @author Olivier Goffart  <ogoffart @ kde.org>
 */
class KDE_EXPORT KNotification : public QObject
{
        Q_OBJECT
public:
	/**
	 * Sometimes, the user may want different notification for the same event, 
	 * depending the source of the event.  Example, you wan to be notified for mails
	 * that arrives in your folder "personal inbox" but not for those in "spam" folder
	 * 
	 * A notification context is a pair of two strings. 
	 * The first string is a key from what the context is.  example "group" or 
	 * "filter" (not translated).
	 * The second is the id of the context. In our example, the group id or the 
	 * filter id in the applications.
	 * Theses string are the one present in the config file, and are in theory not 
	 * shown in the user interface
	 * 
	 * the order of contexts in the list is is importent, most importent context 
	 * should be placed first
	 * 
	 * @todo  contexts doesn't works yet
	 * 
	 * @see event
	 */
	typedef QList< QPair<QString,QString> > ContextList;

	enum NotificationFlags
	{
		/**
		 * When the notification is activated, raise the notification's widget.
		 *
		 * This will change the desktop, raise the window, and switch to the tab.
		 * @todo  doesn't works yet
		 */
		RaiseWidgetOnActivation=0x01,

		/**
		 * The notification will be automatically closed after a timeout.
		 */
		CloseOnTimeout=0x02,
		/**
		 * The notification will be automatically closed if the widget() becomes
		 * activated.
		 * 
		 * If the widget is already activated when the notification occurs, the
		 * notification will be closed after a small timeout.
		 * @todo doesn't works yet
		 */
		CloseWhenWidgetActivated=0x03
	};

	
	~KNotification();

    /**
	 * @brief the widget associated to the notification
	 *
	 * If the widget is destroyed, the notification will be automatically canceled.
	 * If the widget is activated, the notificaiton will be automatically closed if the flags said that
	 *
	 * When the notification is activated, the widget might be raised.
	 * Depending of the configuration, the taskbar entry of the window containing the widget may blink.
	 */
	QWidget *widget();

   signals:
	/**
	 * Emit only when the default activation has occured
	 */
	void activated();
	/**
	 * Emit when an action has been activated.
	 * @param action will be 0 is the default aciton was activated, or any actiton id
	 */
	void activated(unsigned int action);

	/**
	 * Emit when the notification is closed. Both if it's activated or just ignored
	 */
	 void closed();

	/**
	 * The notification has been ignored
	 */
	void ignored();

public slots:
	/**
	 * @brief Active the action specified action
	 * If the action is zero, then the default action is activated
	 */
	void activate(unsigned int action=0);

	/**
	 * close the notification without activate it.
	 *
	 * This will delete the notification
	 */
	void close();

	/**
	 * @brief Raise the widget.
	 * This will change the desktop, activate the window, and the tab if needed.
	 */
	void raiseWidget();



private:
	struct Private;
	Private *d;
	KNotification(QObject *parent=0L);
	/**
	 * recursive function that raise the widget. @p w
	 *
	 * @see raiseWidget()
	 */
	static void raiseWidget(QWidget *w);


private slots:
	void notifyByMessagebox();
	void notifyByPassivePopup(const QPixmap &pix);
	void notifyByExecute(const QString &command, const QString& event,const QString& fromApp, const QString& text,	int winId, int eventId);
	void slotPopupLinkClicked(const QString &);


public:
	/**
	 * @brief emit an event
	 *
	 * A popup may be showed, a sound may be played, depending the config.
	 *
	 * return a KNotification .  You may use that pointer to connect some signals or slot.
	 * the pointer is automatically deleted when the event is closed.
	 * 
	 * Make sure you use one of the CloseOnTimeOut or CloseWhenWidgetActivated, if not,
	 * you have to close yourself the notification.
	 * 
	 * @note the text is shown in a QLabel, you should make sure to escape the html is needed.
	 *
	 * @param eventId is the name of the event
	 * @param text is the text of the notification to show in the popup.
	 * @param pixmap is a picture which may be shown in the popup.  
	 * @param widget is a widget where the notification reports to
	 * @param actions is a list of action texts.
	 * @param contexts is the lists of contexts
	 * @param flags is a bitmask of NotificationsFlags  
	 */
	static KNotification *event( const QString& eventId , const QString& text=QString::null,
			const QPixmap& pixmap=QPixmap(), QWidget *widget=0L,
			const QStringList &actions=QStringList(), ContextList contexts=ContextList() ,
			unsigned int flags=CloseOnTimeout);


	/**
	 * @brief emit a custom event
	 *
	 * @param text is the text of the notification to show in the popup.
	 * @param pixmap is a picture which may be shown in the popup
	 * @param widget is a widget where the notification raports to
	 * @param actions is a list of actions text.
	 * ...
	 */
	static KNotification *userEvent( const QString& text, const QPixmap& pixmap,
				QWidget *widget, QStringList actions,int present, int level,
				const QString &sound, const QString &file,
				const QString &commandline, unsigned int flags);

};



#endif
