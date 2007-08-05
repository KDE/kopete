/*
    kanimatedsystemtrayicon.cpp  -  System Tray Icon that can play movies
				    Designed for Kopete but usable anywhere

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2002-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KANIMATEDSYSTEMTRAYICON_H
#define KANIMATEDSYSTEMTRAYICON_H

#include <ksystemtrayicon.h>

/**
 * \brief %KDE Animated System Tray Icon class
 * Same as @see KSystemTrayIcon, but can handle movies
 * in the form of @see QMovie.
 *
 * @author Charles Connell <charles@connells.org>
 **/

class QMovie;

class KAnimatedSystemTrayIcon : public KSystemTrayIcon
{
		Q_OBJECT
	public:

		/**
		* Construct a system tray icon.
		*
		* The parent widget @p parent has a special meaning:
		* Besides owning the tray window, the parent widget will
		* dissappear from taskbars when it is iconified while the tray
		* window is visible. This is the desired behavior. After all,
		* the tray window @p is the parent's taskbar icon.
		*
		* Furthermore, the parent widget is shown or raised respectively
		* when the user clicks on the tray window with the left mouse
		* button.
		 **/
		explicit KAnimatedSystemTrayIcon ( QWidget* parent = 0 );

		/**
		* Same as above but allows one to define the movie by name that should
		* be used for the system tray icon.
		 */
		KAnimatedSystemTrayIcon ( const QString& movie, QWidget* parent = 0 );

		/**
		* Same as above but allows one to define the movie by QMovie that should
		* be used for the system tray icon. Memory management for the movie will
		* be handled by KAnimatedSystemTrayIcon.
		 */
		KAnimatedSystemTrayIcon ( QMovie* movie, QWidget* parent = 0 );


		~KAnimatedSystemTrayIcon();
		
		/**
		* Set the movie to use. To manipulate the movie (start, stop, pause), call
		* @see movie() and make calls on the QMovie* that it returns.
		 */
		void setMovie (QMovie * movie);
		
		/**
		* Get a pointer to the movie. Use this pointer to manipulate the movie
		* (start, stop, pause).
		* Will return null if no movie has been set
		 */
		const QMovie * movie() const;
		
		/**
		* Is the movie playing?
		* @return Whether or not the movie is playing.
		 */
		bool isPlaying () const;
		
	public slots:
		/**
		* Start the movie.
		* Will do nothing if no movie has been set.
		 */
		void startMovie ();
		
		/**
		* Stop the movie
		* Will do nothing if no movie has been set.
		 */
		void stopMovie ();
		
	private slots:
		void slotNewFrame();

	private:
		class Private;
		KAnimatedSystemTrayIcon::Private* d;
};

#endif
