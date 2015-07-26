#ifndef GNUPGSELECTUSERKEY_H
#define GNUPGSELECTUSERKEY_H

#include <kdialog.h>

namespace Kopete {class MetaContact;}

class GnupgSelectUserKey: public KDialog
{
  Q_OBJECT
public:
  GnupgSelectUserKey(const QString &key, Kopete::MetaContact *mc);
  ~GnupgSelectUserKey();
private:
  Kopete::MetaContact *m_metaContact;
};

#endif