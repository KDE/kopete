/***************************************************************************
     wpdebug.h  -  Debugging header file for WinPopup protocol
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Bruno Rodrigues
    email                : bruno.rodrigues@litux.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WPDEBUG_H
#define WPDEBUG_H

#define WPDEBUGLEVEL 0

#define WPDMETHOD	0
#define WPDPROTOCOL	1
#define WPDDEBUG		2
#define WPDINFO		3
#define WPDERROR		4

#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG(level, text) \
	if(level >= WPDEBUGLEVEL) { kdDebug( 14170 ) << "WinPopup Plugin [" << level << "] [" << __FILE__ << ":" << __LINE__ << "]: " << text << endl; }

#endif

// vim: set noet ts=4 sts=4 sw=4:

