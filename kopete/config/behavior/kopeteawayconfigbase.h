/****************************************************************************
** Form interface generated from reading ui file './kopeteawayconfigbase.ui'
**
** Created: Dom Mai 8 08:12:19 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef KOPETEAWAYCONFIGBASEUI_H
#define KOPETEAWAYCONFIGBASEUI_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QLabel;
class QSpinBox;
class QCheckBox;

class KopeteAwayConfigBaseUI : public QWidget
{
    Q_OBJECT

public:
    KopeteAwayConfigBaseUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~KopeteAwayConfigBaseUI();

    QGroupBox* groupBox2;
    QLabel* textLabel1;
    QSpinBox* rememberedMessages;
    QGroupBox* groupBox3;
    QCheckBox* mUseAutoAway;
    QLabel* TextLabel2;
    QSpinBox* mAutoAwayTimeout;
    QLabel* TextLabel3;
    QCheckBox* mGoAvailable;

protected:
    QVBoxLayout* KopeteAwayConfigBaseUILayout;
    QSpacerItem* spacer5;
    QGridLayout* groupBox2Layout;
    QSpacerItem* spacer6;
    QGridLayout* groupBox3Layout;
    QSpacerItem* spacer7;

protected slots:
    virtual void languageChange();

};

#endif // KOPETEAWAYCONFIGBASEUI_H
