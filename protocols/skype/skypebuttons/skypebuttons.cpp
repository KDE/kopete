
#include <QtGui>
#include <qtbrowserplugin.h>

class SkypeButtons : public QWidget
{
	Q_OBJECT
	Q_CLASSINFO("MIME", "application/x-skype:skype:Skype Buttons")
};

#include "skypebuttons.moc"

QTNPFACTORY_BEGIN("Skype Buttons for Kopete", "Mime Type x-skype for Skype Buttons")
QTNPCLASS(SkypeButtons)
QTNPFACTORY_END()
