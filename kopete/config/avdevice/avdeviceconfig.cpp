//
// C++ Implementation: avdeviceconfig
//
// Description: 
//
//
// Author: Cláudio da Silveira Pinheiro <taupter@gmail.com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "avdeviceconfig.h"
#include "avdeviceconfig_videoconfig.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qhbuttongroup.h>
#include <qspinbox.h>
#include <qcombobox.h>

#include <kdebug.h>
#include <kplugininfo.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kgenericfactory.h>
#include <ktrader.h>
#include <kconfig.h>

#include <qtabwidget.h>

typedef KGenericFactory<AVDeviceConfig, QWidget> KopeteAVDeviceConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_avdeviceconfig, KopeteAVDeviceConfigFactory( "kcm_kopete_avdeviceconfig" ) )

AVDeviceConfig::AVDeviceConfig(QWidget *parent, const char *  name , const QStringList &args)
 : KCModule( KopeteAVDeviceConfigFactory::instance(), parent, args )
{
  (new QVBoxLayout(this))->setAutoAdd(true);
  mAVDeviceTabCtl = new QTabWidget(this, "mAVDeviceTabCtl");

// "General" TAB ============================================================
//  mPrfsGeneral = new BehaviorConfig_General(mBehaviorTabCtl);
  mPrfsVideoDevice = new AVDeviceConfig_VideoDevice(mAVDeviceTabCtl);
//  connect(mPrfsVideoDevice->mShowTrayChk, SIGNAL(toggled(bool)),this, SLOT(slotShowTrayChanged(bool)));
  mAVDeviceTabCtl->addTab(mPrfsVideoDevice, i18n("&Video"));
}


AVDeviceConfig::~AVDeviceConfig()
{
}




/*!
    \fn VideoDeviceConfig::save()
 */
void AVDeviceConfig::save()
{
    /// @todo implement me
}


/*!
    \fn VideoDeviceConfig::load()
 */
void AVDeviceConfig::load()
{
    /// @todo implement me
}

void AVDeviceConfig::slotSettingsChanged(bool){
  emit changed(true);
}

void AVDeviceConfig::slotValueChanged(int){
  emit changed( true );
}
