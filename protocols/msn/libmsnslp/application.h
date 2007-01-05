/*
    application.h - Peer to Peer Application

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__APPLICATION_H
#define CLASS_P2P__APPLICATION_H

#include <qobject.h>

namespace PeerToPeer
{

/**
 * @brief Represents .
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class Application : public QObject
{
	Q_OBJECT

	public :
		/** @brief Creates a new instance of the Application class. */
		Application(QObject *parent);
		virtual ~Application();

		virtual void handleRequest(const Q_UINT32 sessionId, const Q_UINT32 appId, const QString& context);

	signals:
		void accept(const QString& cookie);
		void decline(const QString& cookie);

}; // Application
}

#endif
