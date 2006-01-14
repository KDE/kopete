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

#include <assert.h>

#include <qwindowdefs.h>
#include <qwidget.h>

#include <kdebug.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kxv.h"


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#ifdef HAVE_XSHM
extern "C" {
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
}
#endif

#ifdef HAVE_LIBXV
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#endif

#ifdef HAVE_LIBXVMC
#include <X11/extensions/XvMC.h>
#include <X11/extensions/XvMClib.h>
#endif


KXv::KXv()
{
    xv_adaptors = 0;
    _devs.setAutoDelete(true);
}


KXv::~KXv()
{
    kdDebug() << "KXv::~KXv: Close Xv connection." << endl;
    _devs.clear();
    
#ifdef HAVE_LIBXV
    if (xv_adaptors > 0)
        XvFreeAdaptorInfo((XvAdaptorInfo *)xv_adaptor_info);
#endif
}


KXvDeviceList& KXv::devices()
{
  return _devs;
}


bool KXv::haveXv()
{
#ifndef HAVE_LIBXV
    return false;
#else
    unsigned int tmp;
    if (Success != XvQueryExtension(qt_xdisplay(),
                                    &tmp,
                                    &tmp,
                                    &tmp,
                                    &tmp,
                                    &tmp)) 
        return false;
    
    return true;
#endif
}


KXv* KXv::connect(Drawable d)
{
    KXv *xvptr;
    
    xvptr = new KXv;
    if (!xvptr->init(d)) {
        kdDebug() << "KXv::connect: Xv init failed." << endl;
        delete xvptr;
        return NULL;
    }
    
    kdDebug() << "KXv::connect: Xv init completed." << endl;
    return xvptr;
}


bool KXv::init(Drawable d)
{
#ifndef HAVE_LIBXV
    return false;
#else
    if (Success != XvQueryExtension(qt_xdisplay(), 
                                    &xv_version,
                                    &xv_release,
                                    &xv_request,
                                    &xv_event,
                                    &xv_error)) {
        kdWarning() << "KXv::init: Xv extension not available." << endl;
        return false; 
    }
    
#ifdef HAVE_LIBXVMC
    // Causes crashes for some people.
    //  if (Success == XvMCQueryExtension(qt_xdisplay(),0,0)) {
    //    kdDebug() << "Found XvMC!" << endl;
    //  }
#endif
    
    if (Success != XvQueryAdaptors(qt_xdisplay(),
                                   d,
                                   &xv_adaptors, 
                                   (XvAdaptorInfo **)&xv_adaptor_info)) {
        // Note technically fatal... what to do?
        kdWarning() << "KXv::init: XvQueryAdaptors failed." << endl;
    }
    
    XvAdaptorInfo *ai = (XvAdaptorInfo *)xv_adaptor_info;
    
    for (unsigned int i = 0; i < xv_adaptors; i++) {
        KXvDevice *xvd = new KXvDevice;
        xvd->xv_type = ai[i].type;
        xvd->xv_port = ai[i].base_id;
        xvd->xv_name = ai[i].name;
        xvd->xv_adaptor = i;
        xvd->xv_nvisualformats = ai[i].num_formats;
        xvd->xv_visualformats = ai[i].formats;
        if (ai[i].type & XvInputMask &&
            ai[i].type & XvVideoMask ) {
            kdDebug() << "KXv::init: Xv VideoMask port " << ai[i].base_id << " was found." 
                      << "  Device is: " << ai[i].name << "." << endl;
        }
        if (ai[i].type & XvInputMask &&
            ai[i].type & XvImageMask ) {
            kdDebug() << "KXv::init: Xv ImageMask port " << ai[i].base_id << " was found." 
                      << "  Device is: " << ai[i].name << "." << endl;
        }
        
        if (xvd->init()) {
            _devs.append(xvd);
        } else {
            delete xvd;
        }
    }
    
    return true;
#endif
}

bool KXvDevice::grabStill(QImage* /*pix*/, int /*dw*/, int /*dh*/)
{
#ifndef HAVE_LIBXV
    return false;
#else
    return false;
#endif
}

int KXvDevice::displayImage(QWidget *widget, const unsigned char *const data, int w, int h, int dw, int dh)
{
    if (!widget)
        return -1;
    return displayImage(widget->winId(), data, w, h, 0, 0, w, h, dw, dh);
}

int KXvDevice::displayImage(QWidget *widget, const unsigned char *const data, int w, int h, int x, int y, int sw, int sh, int dw, int dh)
{
    if (!widget)
        return -1;
    return displayImage(widget->winId(), data, w, h, x, y, sw, sh, dw, dh);
}

int KXvDevice::displayImage(Window win, const unsigned char *const data, int w, int h, int dw, int dh)
{
    return displayImage(win, data, w, h, 0, 0, w, h, dw, dh);
}

int KXvDevice::displayImage(Window win, const unsigned char *const data, int w, int h, int x, int y, int sw, int sh, int dw, int dh)
{
#ifndef HAVE_LIBXV
    return -1;
#else
    Q_ASSERT(xv_port != -1);
    
    // Must be a video capable device!
    if (!(xv_type & XvImageMask) || !(xv_type & XvInputMask)) {
        kdWarning() << "KXvDevice::displayImage: This is not a video capable device." << endl;
        return -1;
    }
    
    if (xv_image_w != w || xv_image_h != h || !xv_image)
        rebuildImage(w, h, _shm);

    if (!xv_image)
        return -1;
    
    if (win != xv_last_win && xv_gc) {
        XFreeGC(qt_xdisplay(), xv_gc);
        xv_gc = 0;
    }
    
    if (!xv_gc) {
        xv_last_win = win;   
        xv_gc = XCreateGC(qt_xdisplay(), win, 0, NULL);
    }
    
    int rc = 0;
    Q_ASSERT(xv_image);
    if (!_shm) {
        static_cast<XvImage*>(xv_image)->data = 
            (char *)const_cast<unsigned char*>(data);
        rc = XvPutImage(qt_xdisplay(), xv_port, win, xv_gc, 
                        static_cast<XvImage*>(xv_image), x, y, sw, sh, 0, 0, dw, dh);
    } else {
#ifdef HAVE_XSHM
        memcpy(static_cast<XvImage*>(xv_image)->data, data, static_cast<XvImage*>(xv_image)->data_size);
        rc = XvShmPutImage(qt_xdisplay(), xv_port, win, xv_gc, 
                           static_cast<XvImage*>(xv_image), x, y, sw, sh, 0, 0, dw, dh, 0);
#endif
    }

    XSync(qt_xdisplay(), False);
    return rc;
#endif
}


bool KXvDevice::startVideo(QWidget *w, int dw, int dh)
{
    if (!w) return false;
    return startVideo(w->winId(), dw, dh);
}


bool KXvDevice::startVideo(Window w, int dw, int dh)
{
#ifndef HAVE_LIBXV
    return false;
#else
    int sx = 0, sy = 0, dx = 0, dy = 0, sw = dw, sh = dh;
    
    // Must be a video capable device!
    if (!(xv_type & XvVideoMask) || !(xv_type & XvInputMask)) {
        kdWarning() << "KXvDevice::startVideo: This is not a video capable device." << endl;
        return false;
    }
    
    if (videoStarted) stopVideo();
    
    if (xv_port == -1) {
        kdWarning() << "KXvDevice::startVideo: No xv_port." << endl;
        return false;
    }

    if (w != xv_last_win && xv_gc) {
        XFreeGC(qt_xdisplay(), xv_gc);
        xv_gc = 0;
    }
    
    if (!xv_gc) {
        xv_last_win = w;   
        xv_gc = XCreateGC(qt_xdisplay(), w, 0, NULL);
    }
    
    if (-1 != xv_encoding) {
        sw = ((XvEncodingInfo *)xv_encoding_info)[xv_encoding].width;
        sh = ((XvEncodingInfo *)xv_encoding_info)[xv_encoding].height;
    }
    
    // xawtv does this here:
    //  ng_ratio_fixup(&dw, &dh, &dx, &dy);
    
    kdDebug() << "XvPutVideo: " << qt_xdisplay() 
              << " " << xv_port << " " << w << " " << xv_gc 
              << " " << sx << " " << sy << " " << sw << " " << sh 
              << " " << dx << " " << dy << " " << dw << " " << dh << endl;
    XvPutVideo(qt_xdisplay(), xv_port, w, xv_gc, sx, sy, sw, sh, dx, dy, dw, dh);
    
    videoStarted = true;
    videoWindow = w;
    return true;
#endif
}

bool KXvDevice::stopVideo()
{
#ifndef HAVE_LIBXV
    return false;
#else
    if (!videoStarted)
        return true;
    if (xv_port == -1) {
        kdWarning() << "KXvDevice::stopVideo: No xv_port." << endl;
        return false;
    }

    XvStopVideo(qt_xdisplay(), xv_port, videoWindow);
    videoStarted = false;
    return true;
#endif
}


KXvDevice::KXvDevice()
{
    xv_encoding_info = NULL;
    xv_formatvalues = NULL;
    xv_attr = NULL;
    xv_port = -1;
    xv_encoding = -1;
    xv_name = QString::null;
    xv_type = -1;
    xv_adaptor = -1;
    _shm = false;
#ifdef HAVE_LIBXV
    xv_imageformat = 0x32595559;  // FIXME (YUY2)
#ifdef HAVE_XSHM
    if (!XShmQueryExtension(qt_xdisplay())) {
        _haveShm = false;
    } else {
        _shm = true;
        _haveShm = true;
    }
    xv_shminfo = new XShmSegmentInfo;
#else
    xv_shminfo = 0;
#endif
#endif
    xv_gc = 0;
    xv_last_win = 0;
    videoStarted = false;
    _attrs.setAutoDelete(true);
    xv_image = 0;
    xv_image_w = 320;
    xv_image_h = 200;
}


KXvDevice::~KXvDevice()
{
#ifdef HAVE_LIBXV
    _attrs.clear();
    if (videoStarted) stopVideo();
    if (xv_encoding_info)
        XvFreeEncodingInfo((XvEncodingInfo *)xv_encoding_info);
    XFree(xv_formatvalues);
    XFree(xv_attr);
#ifdef HAVE_XSHM
    delete (XShmSegmentInfo*)xv_shminfo;
#endif
    destroyImage();
#endif
    if (xv_gc)
        XFreeGC(qt_xdisplay(), xv_gc);

#ifdef HAVE_LIBXV
    if (xv_port != -1)
        XvUngrabPort(qt_xdisplay(), xv_port, CurrentTime);
#endif
}


bool KXvDevice::init()
{
#ifndef HAVE_LIBXV
    return false;
#else
    assert(xv_port != -1);   // make sure we were prepped by KXv already.

    if (XvGrabPort(qt_xdisplay(), xv_port, CurrentTime)) {
        kdWarning() << "KXvDevice::init(): Unable to grab Xv port." << endl;
        return false;
    }

    if (Success != XvQueryEncodings(qt_xdisplay(),
                                    xv_port,
                                    &xv_encodings,
                                    (XvEncodingInfo **)&xv_encoding_info)) {
        kdWarning() << "KXvDevice::init: Xv QueryEncodings failed.  Dropping Xv support for this device." << endl;
        return false;
    }
    
    // Package the encodings up nicely
    for (unsigned int i = 0; i < xv_encodings; i++) {
        //kdDebug() << "Added encoding: " << ((XvEncodingInfo *)xv_encoding_info)[i].name << endl;
        _encodingList << ((XvEncodingInfo *)xv_encoding_info)[i].name;
    }
    
    xv_attr = XvQueryPortAttributes(qt_xdisplay(), 
                                    xv_port, 
                                    &xv_encoding_attributes);
    XvAttribute *xvattr = (XvAttribute *)xv_attr;
    kdDebug() << "Attributes for port " << xv_port << endl;
    for (int i = 0; i < xv_encoding_attributes; i++) {
        assert(xvattr);
        kdDebug() << "   -> " << xvattr[i].name 
                  << ((xvattr[i].flags & XvGettable) ? " get" : "")
                  << ((xvattr[i].flags & XvSettable) ? " set" : "")
                  << " Range: " << xvattr[i].min_value
                  << " -> " << xvattr[i].max_value << endl;
        
        KXvDeviceAttribute *xvda = new KXvDeviceAttribute;
        xvda->name = xvattr[i].name;
        xvda->min = xvattr[i].min_value;
        xvda->max = xvattr[i].max_value;
        xvda->flags = xvattr[i].flags;
        _attrs.append(xvda);
    }
    
    XvImageFormatValues  *fo;
    fo = XvListImageFormats(qt_xdisplay(), xv_port, &xv_formats);
    xv_formatvalues = (void *)fo;
    kdDebug() << "Image formats for port " << xv_port << endl;
    for (int i = 0; i < xv_formats; i++) {
        assert(fo);
        QString imout;
        imout.sprintf("   0x%x (%c%c%c%c) %s",
                      fo[i].id,
                      fo[i].id        & 0xff,
                      (fo[i].id >>  8) & 0xff,
                      (fo[i].id >> 16) & 0xff,
                      (fo[i].id >> 24) & 0xff,
                      ((fo[i].format == XvPacked) ? 
                       "Packed" : "Planar"));
        kdDebug() << imout << endl;
    }
    
    kdDebug() << "Disabling double buffering." << endl;
    setAttribute("XV_DOUBLE_BUFFER", 0);

    return true;
#endif
}


bool KXvDevice::supportsWidget(QWidget *w)
{
#ifndef HAVE_LIBXV
    return false;
#else
    for (int i = 0; i < xv_nvisualformats; i++) {
        if (static_cast<XvFormat*>(xv_visualformats)[i].visual_id 
            == static_cast<Visual*>(w->x11Visual())->visualid) {
            return true;
        }
    }
    return false;
#endif
}


bool KXvDevice::isVideoSource()
{
#ifndef HAVE_LIBXV
    return false;
#else
    if (xv_type & XvVideoMask && xv_type & XvInputMask) 
        return true;
    return false;
#endif
}


bool KXvDevice::isImageBackend()
{
#ifndef HAVE_LIBXV
    return false;
#else
    if (xv_type & XvImageMask && xv_type & XvInputMask) 
        return true;
    return false;
#endif
}


const KXvDeviceAttributes& KXvDevice::attributes()
{
    return _attrs;
}


bool KXvDevice::getAttributeRange(const QString& attribute, int *min, int *max)
{
    for (KXvDeviceAttribute *at = _attrs.first(); at != NULL; at = _attrs.next()) {
        if (at->name == attribute) {
            if (min) *min = at->min;
            if (max) *max = at->max;
            return true;
        }
    }
    return false;
}


bool KXvDevice::getAttribute(const QString& attribute, int *val)
{
#ifndef HAVE_LIBXV
    return false;
#else
    for (KXvDeviceAttribute *at = _attrs.first(); at != NULL; at = _attrs.next()) {
        if (at->name == attribute) {
            if (val)
                XvGetPortAttribute(qt_xdisplay(), xv_port, at->atom(), val);
            return true;
        }
    }
    return false;
#endif
}


bool KXvDevice::setAttribute(const QString& attribute, int val)
{
#ifndef HAVE_LIBXV
    return false;
#else
    for (KXvDeviceAttribute *at = _attrs.first(); at != NULL; at = _attrs.next()) {
        if (at->name == attribute) {
            XvSetPortAttribute(qt_xdisplay(), xv_port, at->atom(), val);
            XSync(qt_xdisplay(), False);
            return true;
        }
    }
    return false;
#endif
}


const QString& KXvDevice::name() const
{
    return xv_name;
}


int KXvDevice::port() const 
{
    return xv_port;
}

const QStringList& KXvDevice::encodings() const
{
    return _encodingList;
}

bool KXvDevice::encoding(QString& encoding)
{
#ifndef HAVE_LIBXV
    return false;
#else
    XvEncodingID enc;
    
    for (KXvDeviceAttribute *at = _attrs.first(); at != 0L; at = _attrs.next()) {
        if (at->name == "XV_ENCODING") {
            XvGetPortAttribute(qt_xdisplay(), xv_port, at->atom(), (int*)&enc);
            kdDebug() << "KXvDevice: encoding: " << enc << endl;
            encoding = enc;
            return true;
        }
    }
    return false;
#endif
}

bool KXvDevice::setEncoding(const QString& e)
{
#ifdef HAVE_LIBXV
    for (unsigned int i = 0; i < xv_encodings; i++) {
        if (e == ((XvEncodingInfo *)xv_encoding_info)[i].name) {
            xv_encoding = i;
            return setAttribute("XV_ENCODING", 
                                ((XvEncodingInfo *)xv_encoding_info)[i].encoding_id);
        }
    }
#endif
    return false;
}

bool KXvDevice::videoPlaying() const
{
    return videoStarted;
}


bool KXvDevice::useShm(bool on)
{
#ifndef HAVE_XSHM
    if (on) {
        return false;
    }
#endif
    if (!_haveShm) {
        return false;
    }
    if (_shm != on)
        rebuildImage(xv_image_w, xv_image_h, on);
    if (_haveShm) // This can change in rebuildImage()
        _shm = on;
    return _shm;
}


bool KXvDevice::usingShm() const
{
    return _shm;
}


#include <unistd.h>
void KXvDevice::rebuildImage(int w, int h, bool shm)
{
    if (xv_image) {
        destroyImage();
    }
#ifdef HAVE_LIBXV
    if (!shm) {
        xv_image = (void*)XvCreateImage(qt_xdisplay(), xv_port, xv_imageformat, 
                                        0, w, h);
        if (!xv_image) {
            kdWarning() << "KXvDevice::rebuildImage: XvCreateImage failed." << endl;
        }
    } else {
#ifdef HAVE_XSHM
        memset(xv_shminfo, 0, sizeof(XShmSegmentInfo));
        xv_image = (void*)XvShmCreateImage(qt_xdisplay(), xv_port, xv_imageformat, 
                                           0, w, h, static_cast<XShmSegmentInfo*>(xv_shminfo));
        if (!xv_image) {
            kdWarning() << "KXvDevice::rebuildImage: Error using SHM with Xv! Disabling SHM..." << endl;
            _haveShm = false;
            _shm = false;
            xv_image = (void*)XvCreateImage(qt_xdisplay(), xv_port, xv_imageformat,
                                            0, w, h);
            if (!xv_image) {
                kdWarning() << "KXvDevice::rebuildImage: XvCreateImage failed." << endl;
            }
        } else {
            static_cast<XShmSegmentInfo*>(xv_shminfo)->shmid =
                shmget(IPC_PRIVATE,
                       static_cast<XvImage*>(xv_image)->data_size,
                       IPC_CREAT | 0600);
            static_cast<XShmSegmentInfo*>(xv_shminfo)->shmaddr = 
                (char*)shmat(static_cast<XShmSegmentInfo*>(xv_shminfo)->shmid, 0, 0);
            static_cast<XShmSegmentInfo*>(xv_shminfo)->readOnly = True;
            static_cast<XvImage*>(xv_image)->data =
                static_cast<XShmSegmentInfo*>(xv_shminfo)->shmaddr;
            XShmAttach(qt_xdisplay(), static_cast<XShmSegmentInfo*>(xv_shminfo));
            XSync(qt_xdisplay(), False);
            shmctl(static_cast<XShmSegmentInfo*>(xv_shminfo)->shmid, IPC_RMID, 0);
        }
#endif
    }
    Q_ASSERT(xv_image != 0);
    xv_image_w = w;
    xv_image_h = h;
#endif
}


void KXvDevice::destroyImage()
{
#ifdef HAVE_LIBXV
    if (!_shm) {
        if (xv_image) {
            static_cast<XvImage*>(xv_image)->data = 0;
        }
    } else {
        if (xv_image) {
#ifdef HAVE_XSHM
            shmdt(static_cast<XShmSegmentInfo*>(xv_shminfo)->shmaddr);
#endif
        }
    }
    XFree(xv_image);
    xv_image = 0;
#endif
}


Atom KXvDeviceAttribute::atom()
{
    return XInternAtom(qt_xdisplay(), name.latin1(), False);
}
