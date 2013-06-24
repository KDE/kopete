/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/ 

#ifndef AUTHENTICATIONWIZARD_H
#define AUTHENTICATIONWIZARD_H

/**
  * @author Michael Zanetti
  */

extern "C"{
#include "libotr/proto.h"
}

#include "kopetechatsession.h"

#include "otrlchatinterface.h"

#include "klineedit.h"
#include "kcombobox.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QWizard>
#include <QRadioButton>

class AuthenticationWizard: public QWizard
{
	Q_OBJECT
public:
	explicit AuthenticationWizard(QWidget *parent = 0, ConnContext *context = 0, Kopete::ChatSession *session = 0, bool initiate = true, const QString &question = QString() );
	~AuthenticationWizard();

	static AuthenticationWizard *findWizard(Kopete::ChatSession *session);
	void nextState();
	void finished(bool success, bool trust);
	void aborted();
	
protected:

	virtual int nextId() const;
	virtual bool validateCurrentPage();

private:
	enum { Page_SelectMethod, Page_QuestionAnswer, Page_SharedSecret, Page_ManualVerification, Page_Wait1, Page_Wait2, Page_Final };
	
	ConnContext *context;
	Kopete::ChatSession *session;
	QString question;
	bool initiate;

	QLabel *lQuestion;
	QLabel *lAnswer;
	QLabel *lSecret;
	QLabel *infoLabel;
	QLabel *lFinal;

	QLineEdit *leQuestion;
	QLineEdit *leAnswer;
	QLineEdit *leSecret;

	QRadioButton *rbQA;
	QRadioButton *rbSS;
	QRadioButton *rbMV;

	QComboBox *cbManualAuth;

	QWizardPage *createIntroPage();
	QWizardPage *createQAPage();
	QWizardPage *createSSPage();
	QWizardPage *createMVPage();
	QWizardPage *createFinalPage();

private slots:
	void cancelVerification();
	void updateInfoBox();
	void notificationActivated( unsigned int );
};


class WaitPage: public QWizardPage
{
private:
	bool canContinue;
public:
	WaitPage(const QString &text);
	void ready(){canContinue = true;};
protected:
	virtual bool isComplete() const{return canContinue;};
};

#endif //AUTHENTICATIONWIZARD_H
