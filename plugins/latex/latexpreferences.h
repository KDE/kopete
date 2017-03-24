/*
    Kopete LaTeX Plugin

    Copyright (c) 2004 by Duncan Mac-Vicar Prett   <duncan@kde.org>

    Kopete    (c) 2001-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef LatexPREFERENCES_H
#define LatexPREFERENCES_H

#include <kcmodule.h>
#include <qstring.h>

namespace Ui { class LatexPrefsUI; }

/**
  *@author Duncan Mac-Vicar Prett
  */

class LatexPreferences : public KCModule
{
	Q_OBJECT
public:

	explicit LatexPreferences(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
	~LatexPreferences();

	void save() Q_DECL_OVERRIDE;
	void load() Q_DECL_OVERRIDE;
	void defaults() Q_DECL_OVERRIDE;

private:
	Ui::LatexPrefsUI *m_preferencesDialog;
private slots:
	void slotModified();
};

#endif
