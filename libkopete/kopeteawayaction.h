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

#include <kselectaction.h>

#include "kopete_export.h"

class KActionCollection;
namespace Kopete
{

class OnlineStatus;
class StatusMessage;

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
		 * @param status the OnlineStatus that appears in the signal
		 * @param slot must have the following signature:  ( const OnlineStatus &, const QString & )
		 * @p text, @p pix, @p cut, @p receiver and @p slot are all handled by KSelectAction
		 **/
		AwayAction(const OnlineStatus &status, const QString &text, const QIcon &pix,
				   const KShortcut &cut, const QObject *receiver, const char *slot);

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
		void awayMessageSelected( const Kopete::OnlineStatus& , const Kopete::StatusMessage & );

	private slots:
		void slotAwayChanged();
		void slotSelectAway( int index );

	private:
		class Private;
		Private *d;
};

}

#endif
