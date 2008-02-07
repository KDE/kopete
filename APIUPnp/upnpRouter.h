#include <QtDebug>
#include <QUrl>
#include <QString>
#include <QList>
#include "upnpRouterPrivate.h"
//class UpnpRouterPrivate;

class UPnpRouter
{
	
	/**
	* Retrieves all the routers 
	*
	* @return the list of the routers
	*/
	static QList<UPnpRouter> allRouters();
	
	/**
	*
	* Constructs a router default
	*
	*/	
	static UPnpRouter defaultRouter();

	//2 constructeurs , un pour appelé le routeur via une URL , lautre par son nom c ca?
	// non
	
	// Voila ma solution
	//UPnpRouter();
	// Constructeur avec une url
	/**
	* Constructs a router for a given setting url
	*
	* @param url
	*/
	UPnpRouter(const QUrl &url);
	// Constructeur par copie
	//constructeur par copie ya pa un ~  ?!!!
	/**
	* Constructs a copy of a router
	*
	* @param router the router to copy
	*/
	UPnpRouter(const UPnpRouter &router);
	// A voir ca il n'y a pas d'interré de faire un singleton car les 
	// constructeur sont public 
	// La seule chose qu"il fo faire qu'une fois c'est le 
	UPnpRouter &operator=(const UPnpRouter &router);

	/**
	* Indicates if this router is valid
	*
	* @return true if this router is valid, false otherwise
	*/
	bool isValid() const;
	
	/**
	* Retrieves all description of a router
	*
	* @return string containt a description of a router
	*/
	QString routerDescription() const;

	QUrl url() const;
	void setUrl(const QUrl &url);
	
	bool openPort(quint16 port, const QString &protocol);
	bool closePort(quint16 port);


private:
	static UpnpRouterPrivate *d;
};

