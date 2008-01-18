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

int main()
{
    KComponentData componentData("getwebcam device");  
    // displays all video devices connected
    //TODO we are able to detect a video device used via a usb connection
    foreach (Solid::Device device, Solid::Device::listFromType(Solid::DeviceInterface::Video, QString()))
    {
	  kDebug() << device.udi().toLatin1().constData();
	  kDebug() << device.product().toLatin1().constData();
	  kDebug() << device.vendor().toLatin1().constData();
    }
    return 0;
}
