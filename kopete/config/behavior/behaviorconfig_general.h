/****************************************************************************
** Form interface generated from reading ui file './behaviorconfig_general.ui'
**
** Created: Dom Mai 8 08:12:20 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef BEHAVIORCONFIG_GENERAL_H
#define BEHAVIORCONFIG_GENERAL_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QCheckBox;
class QGroupBox;

class BehaviorConfig_General : public QWidget
{
    Q_OBJECT

public:
    BehaviorConfig_General( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~BehaviorConfig_General();

    QButtonGroup* buttonGroup2;
    QCheckBox* mShowTrayChk;
    QCheckBox* mStartDockedChk;
    QButtonGroup* notifyGroupBox;
    QCheckBox* mBalloonNotifyChk;
    QCheckBox* mTrayflashNotifyChk;
    QCheckBox* mSoundIfAwayChk;
    QCheckBox* mEventIfActive;
    QGroupBox* groupBox1;
    QCheckBox* mUseQueueChk;
    QCheckBox* mAutoConnect;
    QCheckBox* mMouseNavigation;

protected:
    QVBoxLayout* BehaviorConfig_GeneralLayout;
    QSpacerItem* spacer1;
    QVBoxLayout* buttonGroup2Layout;
    QVBoxLayout* notifyGroupBoxLayout;
    QVBoxLayout* groupBox1Layout;

protected slots:
    virtual void languageChange();

};

#endif // BEHAVIORCONFIG_GENERAL_H
