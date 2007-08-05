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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <assert.h>
#include <QX11Info>
#include <qwindowdefs.h>
#include <qwidget.h>

#include <kdebug.h>

#include <config-kopete.h>

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
}


KXv::~KXv()
{
    kDebug() << "KXv::~KXv: Close Xv connection.";
    qDeleteAll(_devs);
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
    if (Success != XvQueryExtension(QX11Info::display(),
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
        kDebug() << "KXv::connect: Xv init failed.";
        delete xvptr;
        return NULL;
    }

    kDebug() << "KXv::connect: Xv init completed.";
    return xvptr;
}


bool KXv::init(Drawable d)
{
#ifndef HAVE_LIBXV
    return false;
#else
    if (Success != XvQueryExtension(QX11Info::display(),
                                    &xv_version,
                                    &xv_release,
                                    &xv_request,
                                    &xv_event,
                                    &xv_error)) {
        kWarning() << "KXv::init: Xv extension not available.";
        return false;
    }

#ifdef HAVE_LIBXVMC
    // Causes crashes for some people.
    //  if (Success == XvMCQueryExtension(QX11Info::display(),0,0)) {
    //    kDebug() << "Found XvMC!";
    //  }
#endif

    if (Success != XvQueryAdaptors(QX11Info::display(),
                                   d,
                                   &xv_adaptors,
                                   (XvAdaptorInfo **)&xv_adaptor_info)) {
        // Note technically fatal... what to do?
        kWarning() << "KXv::init: XvQueryAdaptors failed.";
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
            kDebug() << "KXv::init: Xv VideoMask port " << ai[i].base_id << " was found."
                      << "  Device is: " << ai[i].name << "." << endl;
        }
        if (ai[i].type & XvInputMask &&
            ai[i].type & XvImageMask ) {
            kDebug() << "KXv::init: Xv ImageMask port " << ai[i].base_id << " was found."
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
        kWarning() << "KXvDevice::displayImage: This is not a video capable device.";
        return -1;
    }

    if (xv_image_w != w || xv_image_h != h || !xv_image)
        rebuildImage(w, h, _shm);

    if (!xv_image)
        return -1;

    if (win != xv_last_win && xv_gc) {
        XFreeGC(QX11Info::display(), xv_gc);
        xv_gc = 0;
    }

    if (!xv_gc) {
        xv_last_win = win;
        xv_gc = XCreateGC(QX11Info::display(), win, 0, NULL);
    }

    int rc = 0;
    Q_ASSERT(xv_image);
    if (!_shm) {
        static_cast<XvImage*>(xv_image)->data =
            (char *)const_cast<unsigned char*>(data);
        rc = XvPutImage(QX11Info::display(), xv_port, win, xv_gc,
                        static_cast<XvImage*>(xv_image), x, y, sw, sh, 0, 0, dw, dh);
    } else {
#ifdef HAVE_XSHM
        memcpy(static_cast<XvImage*>(xv_image)->data, data, static_cast<XvImage*>(xv_image)->data_size);
        rc = XvShmPutImage(QX11Info::display(), xv_port, win, xv_gc,
                           static_cast<XvImage*>(xv_image), x, y, sw, sh, 0, 0, dw, dh, 0);
#endif
    }

    XSync(QX11Info::display(), False);
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
        kWarning() << "KXvDevice::startVideo: This is not a video capable device.";
        return false;
    }

    if (videoStarted) stopVideo();

    if (xv_port == -1) {
        kWarning() << "KXvDevice::startVideo: No xv_port.";
        return false;
    }

    if (w != xv_last_win && xv_gc) {
        XFreeGC(QX11Info::display(), xv_gc);
        xv_gc = 0;
    }

    if (!xv_gc) {
        xv_last_win = w;
        xv_gc = XCreateGC(QX11Info::display(), w, 0, NULL);
    }

    if (-1 != xv_encoding) {
        sw = ((XvEncodingInfo *)xv_encoding_info)[xv_encoding].width;
        sh = ((XvEncodingInfo *)xv_encoding_info)[xv_encoding].height;
    }

    // xawtv does this here:
    //  ng_ratio_fixup(&dw, &dh, &dx, &dy);

    kDebug() << "XvPutVideo: " << QX11Info::display()
              << " " << xv_port << " " << w << " " << xv_gc
              << " " << sx << " " << sy << " " << sw << " " << sh
              << " " << dx << " " << dy << " " << dw << " " << dh << endl;
    XvPutVideo(QX11Info::display(), xv_port, w, xv_gc, sx, sy, sw, sh, dx, dy, dw, dh);

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
        kWarning() << "KXvDevice::stopVideo: No xv_port.";
        return false;
    }

    XvStopVideo(QX11Info::display(), xv_port, videoWindow);
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
    xv_name.clear();
    xv_type = -1;
    xv_adaptor = -1;
    _shm = false;
#ifdef HAVE_LIBXV
    xv_imageformat = 0x32595559;  // FIXME (YUY2)
#ifdef HAVE_XSHM
    if (!XShmQueryExtension(QX11Info::display())) {
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
    xv_image = 0;
    xv_image_w = 320;
    xv_image_h = 200;
}


KXvDevice::~KXvDevice()
{
#ifdef HAVE_LIBXV
    qDeleteAll(_attrs);
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
        XFreeGC(QX11Info::display(), xv_gc);

#ifdef HAVE_LIBXV
    if (xv_port != -1)
        XvUngrabPort(QX11Info::display(), xv_port, CurrentTime);
#endif
}


bool KXvDevice::init()
{
#ifndef HAVE_LIBXV
    return false;
#else
    assert(xv_port != -1);   // make sure we were prepped by KXv already.

    if (XvGrabPort(QX11Info::display(), xv_port, CurrentTime)) {
        kWarning() << "KXvDevice::init(): Unable to grab Xv port.";
        return false;
    }

    if (Success != XvQueryEncodings(QX11Info::display(),
                                    xv_port,
                                    &xv_encodings,
                                    (XvEncodingInfo **)&xv_encoding_info)) {
        kWarning() << "KXvDevice::init: Xv QueryEncodings failed.  Dropping Xv support for this device.";
        return false;
    }

    // Package the encodings up nicely
    for (unsigned int i = 0; i < xv_encodings; i++) {
        //kDebug() << "Added encoding: " << ((XvEncodingInfo *)xv_encoding_info)[i].name;
        _encodingList << ((XvEncodingInfo *)xv_encoding_info)[i].name;
    }

    xv_attr = XvQueryPortAttributes(QX11Info::display(),
                                    xv_port,
                                    &xv_encoding_attributes);
    XvAttribute *xvattr = (XvAttribute *)xv_attr;
    kDebug() << "Attributes for port " << xv_port;
    for (int i = 0; i < xv_encoding_attributes; i++) {
        assert(xvattr);
        kDebug() << "   -> " << xvattr[i].name
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
    fo = XvListImageFormats(QX11Info::display(), xv_port, &xv_formats);
    xv_formatvalues = (void *)fo;
    kDebug() << "Image formats for port " << xv_port;
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
        kDebug() << imout;
    }

    kDebug() << "Disabling double buffering.";
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
    for (KXvDeviceAttributes::iterator it = _attrs.begin(); it != _attrs.end(); ++it) {
        if ((*it)->name == attribute) {
            if (min) *min = (*it)->min;
            if (max) *max = (*it)->max;
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
    for (KXvDeviceAttributes::iterator it = _attrs.begin(); it != _attrs.end(); ++it) {
        if ((*it)->name == attribute) {
            if (val)
                XvGetPortAttribute(QX11Info::display(), xv_port, (*it)->atom(), val);
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
    for (KXvDeviceAttributes::iterator it = _attrs.begin(); it != _attrs.end(); ++it) {
        if ((*it)->name == attribute) {
            XvSetPortAttribute(QX11Info::display(), xv_port, (*it)->atom(), val);
            XSync(QX11Info::display(), False);
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

    for (KXvDeviceAttributes::iterator it = _attrs.begin(); it != _attrs.end(); ++it) {
        if ((*it)->name == "XV_ENCODING") {
            XvGetPortAttribute(QX11Info::display(), xv_port, (*it)->atom(), (int*)&enc);
            kDebug() << "KXvDevice: encoding: " << enc;
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
#include <QX11Info>
void KXvDevice::rebuildImage(int w, int h, bool shm)
{
    if (xv_image) {
        destroyImage();
    }
#ifdef HAVE_LIBXV
    if (!shm) {
        xv_image = (void*)XvCreateImage(QX11Info::display(), xv_port, xv_imageformat,
                                        0, w, h);
        if (!xv_image) {
            kWarning() << "KXvDevice::rebuildImage: XvCreateImage failed.";
        }
    } else {
#ifdef HAVE_XSHM
        memset(xv_shminfo, 0, sizeof(XShmSegmentInfo));
        xv_image = (void*)XvShmCreateImage(QX11Info::display(), xv_port, xv_imageformat,
                                           0, w, h, static_cast<XShmSegmentInfo*>(xv_shminfo));
        if (!xv_image) {
            kWarning() << "KXvDevice::rebuildImage: Error using SHM with Xv! Disabling SHM...";
            _haveShm = false;
            _shm = false;
            xv_image = (void*)XvCreateImage(QX11Info::display(), xv_port, xv_imageformat,
                                            0, w, h);
            if (!xv_image) {
                kWarning() << "KXvDevice::rebuildImage: XvCreateImage failed.";
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
            XShmAttach(QX11Info::display(), static_cast<XShmSegmentInfo*>(xv_shminfo));
            XSync(QX11Info::display(), False);
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
    return XInternAtom(QX11Info::display(), name.toLatin1(), False);
}
