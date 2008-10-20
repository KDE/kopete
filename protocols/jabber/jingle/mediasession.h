#ifndef MEDIA_SESSION_H
#define MEDIA_SESSION_H

#include <QObject>

class MediaManager;
class MediaSession : public QObject
{
	Q_OBJECT
public:
	MediaSession(MediaManager *mediaManager, const QString& codecName);
	~MediaSession();

	void setSamplingRate(int sr);
	void setQuality(int q);
	bool start();
	void write(const QByteArray& sData);
	QByteArray read() const;

public slots:
	void slotReadyRead();
	void slotEncoded();
	void slotDecoded();

signals:
	void readyRead(int);

private:
	class Private;
	Private *d;
};

#endif
