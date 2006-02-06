/*
    detector.h
 
    Copyright (c) 2004-2006 by Heiko Schaefer        <heiko@rangun.de>
 
    Kopete    (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef DETECTOR_H
#define DETECTOR_H

class IConnector;

/**
 * @brief Detector interface to find out if there is a connection to the internet.
 *
 * Subclasses should implement the specific ways to check for an internet 
 * connection
 *
 * @author Heiko Sch&auml;fer <heiko@rangun.de>
 *
 */

class Detector {

    Detector(const Detector&);
    Detector& operator=(const Detector&);

public:
    /**
     * @brief Creates an <code>Detector</code> instance.
     *
     * @param connector A connector to send feedback to the calling object
     */
	Detector(IConnector * connector) : m_connector(connector) {}

    /**
     * @brief Destroys an <code>Detector</code> instance.
     *
     */
	virtual ~Detector() {}

    virtual void checkStatus() const = 0;
	
	virtual void smpppdServerChange() {}

protected:
    IConnector * m_connector;
};

#endif
