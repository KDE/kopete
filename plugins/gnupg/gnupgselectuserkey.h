#ifndef GNUPGSELECTUSERKEY_H
#define GNUPGSELECTUSERKEY_H

#include <kdialog.h>
#include <QLineEdit>
#include <QObject>

namespace Kopete {class MetaContact;}

class GnupgSelectUserKey: public KDialog
{
  Q_OBJECT
public:
  GnupgSelectUserKey(Kopete::MetaContact *mc);
  virtual void save(QString *tempKey);
  ~GnupgSelectUserKey();
public slots:
  void loadFile();
private:
  QLineEdit *pathKey;
  Kopete::MetaContact *m_metaContact;
};

#endif