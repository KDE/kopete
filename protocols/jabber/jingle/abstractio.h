#ifndef ABSTRACT_IO_H
#define ABSTRACT_IO_H

#include <QObject>
#include <QByteArray>

class AbstractIO : public QObject
{
	Q_OBJECT
public :
	AbstractIO();
	virtual ~AbstractIO();

	virtual bool start() {return false;}

	virtual void encode(const QByteArray& data);
	virtual void decode(const QByteArray& data);

	virtual QByteArray encodedData() const;
	virtual QByteArray decodedData() const;
	virtual int tsValue(); //FIXME:should it be const ?

	// Returns the Period Size in bytes
	// Period Size is the size of a frame which can be decoded in one time.
	virtual int frameSizeBytes();

signals:
	void encoded();
	void decoded();
};

#endif
