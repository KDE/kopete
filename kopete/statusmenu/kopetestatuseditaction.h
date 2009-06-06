/*
    kopetestatuseditaction.h - Kopete Status Edit Action

    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETESTATUSEDITACTION_H
#define KOPETESTATUSEDITACTION_H

#include <QtGui/QWidgetAction>

#include "kopete_export.h"

namespace Kopete
{
	class StatusMessage;

	namespace UI
	{
		class StatusEditWidget;

		class KOPETE_STATUSMENU_EXPORT StatusEditAction: public QWidgetAction
		{
			Q_OBJECT
		public:
			/**
			 * StatusEditAction constructor
			 * Action for changing status title and message
			 **/
			StatusEditAction( QObject *parent );

			/**
			 * Returns Kopete::StatusMessage
			 **/
			Kopete::StatusMessage statusMessage() const;

			/**
			 * Set status message to @p statusMessage
			 **/
			void setStatusMessage( const Kopete::StatusMessage& statusMessage );

		Q_SIGNALS:
			/**
			 * This signal emitted after status message was changed
			 **/
			void statusChanged( const Kopete::StatusMessage& statusMessage );

		private Q_SLOTS:
			void changeClicked();
			void clearClicked();

		private:
			void hideMenu();

			StatusEditWidget *mStatusEditWidget;
		};

	}

}
#endif
