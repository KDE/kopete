/*
    kopetehistorydialog.h - Kopete Away Action

    Copyright (c) 2003     Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEAWAYACTION_H
#define KOPETEAWAYACTION_H

#include <kdeversion.h>
#include <kactionclasses.h>
#include <kaction.h>

#include "kopete_export.h"

namespace Kopete
{

class OnlineStatus;

/**
 * @class Kopete::AwayAction
 *
 * Kopete::AwayAction is a KAction that lets you select an away message
 * from the list of predefined away messages, or enter a custom one.
 *
 * @author Jason Keirstead   <jason@keirstead.org>
 */
class KOPETE_EXPORT AwayAction : public KSelectAction
{
	Q_OBJECT
	public:
		/**
		 * Constructor
		 * @p text, @p pix, @p cut, @p receiver, @p slot, @p parent and
		 * @p name are all handled by KSelectAction.
		 **/
		AwayAction(const QString &text, const QIconSet &pix,
				   const KShortcut &cut, const QObject *receiver, const char *slot,
				   QObject *parent, const char *name = 0);

		/**
		 * Constructor
		 * @param status the OnlineStatus that appears in the signal
		 * @param slot must have the following signature:  ( const OnlineStatus &, const QString & )
		 * @p text, @p pix, @p cut, @p receiver, @p slot, @p parent and
		 * @p name are all handled by KSelectAction.
		 **/
		AwayAction(const OnlineStatus &status, const QString &text, const QIconSet &pix,
				   const KShortcut &cut, const QObject *receiver, const char *slot,
				   QObject *parent, const char *name = 0);

		/**
	 	 * Destructor.
	 	 */
		~AwayAction();

	signals:
		/**
		* @brief Emits when the user selects an away message
		*/
		void awayMessageSelected( const QString & );

		 /**
		  * same as above, but with the saved status
		  */
		void awayMessageSelected( const Kopete::OnlineStatus& , const QString & );

	private slots:
		void slotAwayChanged();
		void slotSelectAway( int index );

	private:
		class Private;
		Private *d;
};

}

#endif
