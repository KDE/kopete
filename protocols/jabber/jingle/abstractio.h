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

signals:
	void encoded();
	void decoded();
};

#endif
