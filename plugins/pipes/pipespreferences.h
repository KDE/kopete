/*
    pipespreferences.h

    Copyright (c) 2007      by Charles Connell <charles@connells.org>

    Kopete    (c) 2007      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PIPESPREFERENCES_H
#define PIPESPREFERENCES_H

#include <kcmodule.h>
#include <QVariantList>
#include <QUuid>

namespace Ui { class PipesPrefsUI; }
class PipesModel;

/**
 * Preference widget for Pipes plugin
 * @author Charles Connell <charles@connells.org>
 */

class PipesPreferences : public KCModule
{
	Q_OBJECT

	public:
		explicit PipesPreferences(QWidget *parent = 0, const QVariantList &args = QVariantList());
		~PipesPreferences();

		/*
		 * Save pipes to file; QUuid's are used to identify them
		 */
		void save();
		
		/*
		 * Load pipes by QUuid
		 */
		void load();
		
		QSize sizeHint() const { return QSize ( 600, 350 ); }

	private slots:
		/*
		 * Add a row to the model
		 */
		void slotAdd();
		
		/*
		 * Remove a row from the model
		 */
		void slotRemove();
		
		/*
		 * Resize tableview
		 */
		void slotListChanged();

	private:
		Ui::PipesPrefsUI *mPrefs;
		PipesModel * mModel;
};

#endif
