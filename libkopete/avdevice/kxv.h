/* 
 *   KDE Xv interface
 *
 *   Copyright (C) 2001 George Staikos (staikos@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __KXV_H
#define __KXV_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>

class QWidget;
class QImage;

class KXvPrivate;
class KXvDevice;
class KXvDevicePrivate;

typedef QPtrList<KXvDevice> KXvDeviceList;


class KXv
{
public:
    ~KXv();
    
    /*
     *   To get access to the Xv extension, you call this method.  It will return
     *   a KXv* object on success, or NULL if it can't connect.
     *
     *   d is typically the Window ID
     */
    static KXv *connect(Drawable d);
    
    /*
     *   True if we can connect to the Xv extension.
     */
    static bool haveXv();
    
    /*
     *   Return the list of Xv devices
     */
    KXvDeviceList& devices();
    
protected:
    KXv();
    bool init(Drawable d);
    
    /***   XV info   ***/
    unsigned int xv_version, xv_release, xv_request, xv_event, xv_error;
    unsigned int xv_adaptors;
    void *xv_adaptor_info;
    
    KXvDeviceList _devs;
    
private:
    KXvPrivate *d;
};



class KXvDeviceAttribute
{
public:
    QString name;
    int min;
    int max;
    int flags;
    
    Atom atom();
};

typedef QPtrList<KXvDeviceAttribute> KXvDeviceAttributes;


class KXvDevice
{
    friend class KXv;
public:

    KXvDevice();
    ~KXvDevice();
    
    /*
     *   return the list of known attributes
     */
    const KXvDeviceAttributes& attributes();
    
    /*
     *   return the range for a given attribute
     */
    bool getAttributeRange(const QString& attribute, int *min, int *max);
    
    /*
     *   get the current value of a given attribute
     */
    bool getAttribute(const QString& attribute, int *val);
    
    /*
     *   set the current value of a given attribute
     */
    bool setAttribute(const QString& attribute, int val);
    
    bool grabStill(QImage *pix, int dw, int dh);
    
    /*
     *   True if this device can operate on the given widget
     */
    bool supportsWidget(QWidget *w);
    
    /*
     *   Display the given image with Xv.
     */
    int displayImage(QWidget *widget, const unsigned char *const data, int w, int h, int dw, int dh);
    int displayImage(Window win, const unsigned char *const data, int w, int h, int dw, int dh);

    /*
     *   Display a portion of the given image with Xv.
     */
    int displayImage(QWidget *widget, const unsigned char *const data, int w, int h, int x, int y, int sw, int sh, int dw, int dh);
    int displayImage(Window win, const unsigned char *const data, int w, int h, int x, int y, int sw, int sh, int dw, int dh);
    
    /*
     *   Start a video stream in widget w, width dw, height dh
     */
    bool startVideo(QWidget *w, int dw, int dh);
    bool startVideo(Window w, int dw, int dh);
    
    /*
     *   Is the video playing
     */
    bool videoPlaying() const;
    
    /*
     *   Stop video stream
     */
    bool stopVideo();
    
    /*
     *   True if this is an image output backend (video card)
     */
    bool isImageBackend();
    
    /*
     *   True if this is a video source
     */
    bool isVideoSource();
    
    /*
     *   Name of the device
     */
    const QString& name() const;
    
    /*
     *   The Xv port for this device
     */
    int port() const;
    
    /*
     *   The list of encodings/norms available
     */
    const QStringList& encodings() const;
    
    /*
     *   get encoding
     */
    bool encoding(QString& encoding);
    
    /*
     *   Set the encoding to the given one.  This should be taken from the list.
     */
    bool setEncoding(const QString& e);
    
    /*
     *   Set the image format. (ex YUV)
     */
    int setImageFormat(int format);
    
    /*
     *   Get the current image format
     */
    int imageFormat() const;
    
    /*
     *   Use SHM for PutImage if available
     */
    bool useShm(bool on);
    
    /*
     *   Is SHM being used?
     */
    bool usingShm() const;

    
protected:
    bool init();
    
    bool _shm;
    KXvDeviceAttributes _attrs;
    
    int xv_type, xv_adaptor;
    QString xv_name;
    int xv_port;
    unsigned int xv_encodings;
    int xv_encoding;
    void *xv_encoding_info;
    int xv_encoding_attributes;
    void *xv_attr;
    GC xv_gc;
    Window xv_last_win;
    
    QStringList _encodingList;
    
    int xv_formats;
    void *xv_formatvalues;
    
    int xv_nvisualformats;
    void *xv_visualformats;  // XvFormat*
    
    bool videoStarted;
    Window videoWindow;
    
    long xv_imageformat;
    
    void *xv_shminfo;
    void *xv_image;
    int xv_image_w;
    int xv_image_h;
    bool _haveShm;

    
private:
    KXvDevicePrivate *d;
    
    void rebuildImage(int w, int h, bool shm);
    void destroyImage();
};


#endif

