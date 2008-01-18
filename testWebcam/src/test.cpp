//QT specific includes
#include <QList>
#include <QString>
 
//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/video.h>
 
//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main()//int args, char **argv)
{
    KComponentData componentData("getwebcam device");  
 //get a list of all devices that are AudioInterface
    //foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()))
    foreach (Solid::Device device, Solid::Device::allDevices())
    {
      QString str = QString("Video");
      QString st = QString(device.product().toLatin1().constData());
      if (st.contains(str,Qt::CaseInsensitive))
      { 
	  kDebug() << device.udi().toLatin1().constData();
	  kDebug() << device.product().toLatin1().constData();
	  kDebug() << device.vendor().toLatin1().constData();
      }
    }
    printf("###### robert\n");
    return 0;
}
