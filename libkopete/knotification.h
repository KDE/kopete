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



#include <qobject.h>
#include <qstringlist.h>
#include <kdemacros.h>

class QWidget;
namespace Kopete { class MetaContact; }

/**
 * KNotification is used to notify some event to the user.
 * 
 * use the static funciton event() to fire an event
 *
 * the returned KNotification pointer may be used to connect signals or slots
 */
class KDE_EXPORT KNotification : public QObject
{
Q_OBJECT;
public:
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
	 * @param action will be 0 is the default aciton was activated, or any aciton id
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



private:
	struct Private;
	Private *d;
	KNotification();

private slots:
	void notifyByMessagebox();
	void notifyByPassivePopup(const QPixmap &pix);
	void notifyByExecute(const QString &command, const QString& event,const QString& fromApp, const QString& text,	int winId, int eventId);
	void slotPopupLinkClicked(const QString &);


public:
	/**
	 * @brief emit an event
	 *
	 * @Note the text is shown in a QLabel, you should make sure to escape the html is needed.
	 *
	 * @param eventId is the name of the event
	 * @param text is the text of the notification which may be shown in the popup.
	 * @param pixmap is a picture which may be shown in the popup
	 * @param widget is a widget where the notification raports to
	 * @param actions is a list of actions text.
	 */
	static KNotification *event( const QString& eventId , const QString& text=QString::null,
								 const QPixmap& pixmap=0, QWidget *widget=0L,
								 const QStringList &actions=QStringList());


	/**
	 * @brief emit an custom event
	 *
	 * @param text is the text of the notification which may be shown in the popup.
	 * @param pixmap is a picture which may be shown in the popup
	 * @param widget is a widget where the notification raports to
	 * @param actions is a list of actions text.
	 * ...
	 */
	static KNotification *userEvent( const QString& text,
				const QPixmap& pixmap, QWidget *widget,
				QStringList actions,int present, int level,
				const QString &sound, const QString &file, const QString &commandline);



	/**
	* @todo  find a proper way to do contect-depedent notifications
	*/
	static KNotification *event( Kopete::MetaContact *mc, const QString& eventId , const QString& text=QString::null,
								const QPixmap& pixmap=0, QWidget *widget=0L,
								const QStringList &actions=QStringList());
};



#endif
