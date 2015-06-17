#ifndef CRYPTOGRAPHY2PREFERENCES_H
#define CRYPTOGRAPHY2PREFERENCES_H

#include <kcmodule.h>
#include <QVariantList>
#include <QtCrypto/QtCrypto>
#include <QtCrypto/QtCrypto>

class QCheckBox;

namespace QCA

class Cryptography2Preferences: public KCModule
{
  Q_OBJECT
public:
  explicit Cryptography2Preferences(QWidget *parent=0, const QVariantList &args = QVariantList() );
  virtual ~Cryptography2Preferences();
  virtual void save();
  virtual void load();
  virtual void defaults();
  
private:
  QCheckBox *checkBox;
  QCA::Initializer init;
};

#endif