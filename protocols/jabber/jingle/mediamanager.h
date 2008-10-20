#ifndef MEDIA_MANAGER_H
#define MEDIA_MANAGER_H

#include <QObject>
#include <QByteArray>

class AlsaIO;
class MediaManager : public QObject
{
	Q_OBJECT
public:
	MediaManager(QString, QString);
	~MediaManager();
	AlsaIO *alsaIn() const;
	AlsaIO *alsaOut() const;

	bool start();

	QByteArray read();
	void write(const QByteArray& data);

private:
	class Private;
	Private *d;
};

#endif
