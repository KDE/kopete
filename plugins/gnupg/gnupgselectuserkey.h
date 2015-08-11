#ifndef GNUPGSELECTUSERKEY_H
#define GNUPGSELECTUSERKEY_H

#include <kdialog.h>
#include <QObject>

namespace Kopete {class MetaContact;}

class GnupgSelectUserKey: public KDialog
{
  Q_OBJECT
public:
  GnupgSelectUserKey();
  ~GnupgSelectUserKey();
private:
  Kopete::MetaContact *m_metaContact;
};

#endif