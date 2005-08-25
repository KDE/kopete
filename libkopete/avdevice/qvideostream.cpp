/*
 *
 * Copyright (C) 2002 George Staikos <staikos@kde.org>
 *               2004 Dirk Ziegelmeier <dziegel@gmx.de>
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

#include "qvideostream.h"
#include <qevent.h>
#include <qimage.h>
#include <qtimer.h>

#include <kdebug.h>
#include "kxv.h"

#include <sys/types.h>
#include <X11/Xutil.h>

#ifdef HAVE_XSHM
extern "C" {
#include <sys/shm.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
}
#endif

#ifdef HAVE_GL
class QVideoStreamGLWidget : public QGLWidget
{
public:
    QVideoStreamGLWidget(QWidget* parent = 0, const char* name = 0);
    virtual ~QVideoStreamGLWidget();

    void setInputSize(const QSize& sz);
    void display(const unsigned char *const img, int x, int y, int sw, int sh);

private:
    virtual void resizeGL(int w, int h);
    void initializeGL();    

    virtual bool eventFilter( QObject *o, QEvent *e );
    void calc(QPoint& p, QPoint& v);
    

    QSize _inputSize;
    GLuint _tex;
    int _tw, _th;
    QWidget* _w;
    int _maxGL;
    QSize _sz;
    bool _glfun;
    QPoint _ul, _ur, _ll, _lr;
    QPoint _vul, _vur, _vll, _vlr;
    QTimer* _glfunTimer;
};
#endif

class QVideoStreamPrivate
{
public:
    QVideoStreamPrivate();
    ~QVideoStreamPrivate();
	KXv *xvHandle;
	KXvDevice *xvdev;
	XImage *xim;
	GC gc;
#ifdef HAVE_GL
    QVideoStreamGLWidget* glwidget;
#endif
#ifdef HAVE_XSHM
	XShmSegmentInfo shmh;
#endif
};

QVideoStreamPrivate::QVideoStreamPrivate()
{
	xvHandle = 0;
	xim = 0;
}

QVideoStreamPrivate::~QVideoStreamPrivate()
{
	delete xvHandle;
}

QVideoStream::QVideoStream(QWidget *widget, const char* name)
    : QObject(widget, name),
      d(new QVideoStreamPrivate),
      _w(widget),
      _methods(METHOD_NONE),
      _method(METHOD_NONE),
      _format(FORMAT_NONE),
      _init(false)
{
    int dummy;
    unsigned int dummy2;
    findDisplayProperties(_xFormat, dummy, dummy2, dummy);

	_methods = (VideoMethod)(_methods | METHOD_X11);

#ifdef HAVE_XSHM
	if (XShmQueryExtension(_w->x11Display())) {
		_methods = (VideoMethod)(_methods | METHOD_XSHM);
	}
#endif

	if (KXv::haveXv()) {
		_methods = (VideoMethod)(_methods | METHOD_XV);
#ifdef HAVE_XSHM
		_methods = (VideoMethod)(_methods | METHOD_XVSHM);
#endif
	}

#ifdef HAVE_GL
    if (QGLFormat::hasOpenGL()) {
        _methods = (VideoMethod)(_methods | METHOD_GL);
    }
#endif
    
	d->gc = XCreateGC(_w->x11Display(), _w->winId(), 0, NULL);
}

QVideoStream::~QVideoStream()
{
	deInit();
	XFreeGC(_w->x11Display(), d->gc);
	delete d;
}

void QVideoStream::deInit()
{
	if (!_init)
		return;

	_init = false;
    _format = FORMAT_NONE;

    Q_ASSERT(_methods & _method);
    if (!(_methods & _method))
        return;
    
	switch (_method) {
	case METHOD_XSHM:
#ifdef HAVE_XSHM
		XShmDetach(_w->x11Display(), &(d->shmh));
		XDestroyImage(d->xim);
		d->xim = 0;
		shmdt(d->shmh.shmaddr);
#endif
        break;
	case METHOD_X11:
		delete[] d->xim->data;
		d->xim->data = 0;
		XDestroyImage(d->xim);
		d->xim = 0;
        break;
	case METHOD_XVSHM:
	case METHOD_XV:
		delete d->xvHandle;
		d->xvHandle = 0;
        break;
	case METHOD_GL:
#ifdef HAVE_GL
        delete d->glwidget;
#endif
        break;
	default:
		Q_ASSERT(0);
		return;
	}
}

void QVideoStream::init()
{
    Q_ASSERT(_methods & _method);
    if (!(_methods & _method))
        return;
    
	switch (_method) {
	case METHOD_XSHM:
        {
#ifdef HAVE_XSHM
            if ( !_inputSize.isValid() ) {
                kdWarning() << "QVideoStream::init() (XSHM): Unable to initialize due to invalid input size." << endl;
                return;
            }
            
            memset(&(d->shmh), 0, sizeof(XShmSegmentInfo));
            d->xim = XShmCreateImage(_w->x11Display(),
                                     (Visual*)_w->x11Visual(),
                                     _w->x11Depth(),
                                     ZPixmap, 0, &(d->shmh),
                                     _inputSize.width(),
                                     _inputSize.height());
            d->shmh.shmid = shmget(IPC_PRIVATE,
                                   d->xim->bytes_per_line*d->xim->height,
                                   IPC_CREAT|0600);
            d->shmh.shmaddr = (char *)shmat(d->shmh.shmid, 0, 0);
            d->xim->data = (char*)d->shmh.shmaddr;
            d->shmh.readOnly = False;
            Status s = XShmAttach(_w->x11Display(), &(d->shmh));
            if (s) {
                XSync(_w->x11Display(), False);
                shmctl(d->shmh.shmid, IPC_RMID, 0);
                _format = _xFormat;
                _init = true;
            } else {
                kdWarning() << "XShmAttach failed!" << endl;
                XDestroyImage(d->xim);
                d->xim = 0;
                shmdt(d->shmh.shmaddr);
            }
#endif
        }
        break;
	case METHOD_X11:
		if ( !_inputSize.isValid() ) {
            kdWarning() << "QVideoStream::init() (X11): Unable to initialize due to invalid input size." << endl;
			return;
        }
        
        d->xim = XCreateImage(_w->x11Display(),
                              (Visual*)_w->x11Visual(),
                              _w->x11Depth(),
                              ZPixmap, 0, 0,
                              _inputSize.width(),
                              _inputSize.height(),
                              32, 0);
        
		d->xim->data = new char[d->xim->bytes_per_line*_inputSize.height()];
        _format = _xFormat;
		_init = true;
        break;
	case METHOD_XVSHM:
	case METHOD_XV:
		{
            if (d->xvHandle)
                delete d->xvHandle;
            
            d->xvHandle = KXv::connect(_w->winId());
            KXvDeviceList& xvdl(d->xvHandle->devices());
            KXvDevice *xvdev = NULL;
            
            for (xvdev = xvdl.first(); xvdev; xvdev = xvdl.next()) {
                if (xvdev->isImageBackend() &&
                    xvdev->supportsWidget(_w)) {
                    d->xvdev = xvdev;
                    d->xvdev->useShm(_method == METHOD_XVSHM);
                    _format = FORMAT_YUYV;
                    _init = true;
                    break;
                }
            }
            
            if (!_init) {
                delete d->xvHandle;
                d->xvHandle = 0;
            }
        }
        break;
	case METHOD_GL:
#ifdef HAVE_GL
        d->glwidget = new QVideoStreamGLWidget(_w, "QVideoStreamGLWidget");
        d->glwidget->resize(_w->width(), _w->height());
        d->glwidget->show();
        _format = FORMAT_BGR24;
        _init = true;
#endif
        break;
    default:
        Q_ASSERT(0);
		return;
	}
}

bool QVideoStream::haveMethod(VideoMethod method) const
{
	return _methods & method;
}

QVideo::VideoMethod QVideoStream::method() const
{
	return _method;
}

QVideo::VideoMethod QVideoStream::setMethod(VideoMethod method)
{
	if (_methods & method) {
		deInit();
		_method = method;
		init();
	}
    
    return _method;
}

QSize QVideoStream::maxSize() const
{
    return _size;
}

int QVideoStream::maxWidth() const
{
    return _size.width();
}

int QVideoStream::maxHeight() const
{
    return _size.height();
}

QSize QVideoStream::size() const
{
	return _size;
}

int QVideoStream::width() const
{
	return _size.width();
}

int QVideoStream::height() const
{
	return _size.height();
}

QSize QVideoStream::setSize(const QSize& sz)
{
	_size = sz;
    return _size;
}

int QVideoStream::setWidth(int width)
{
	if (width < 0)
		width = 0;
	if (width > maxWidth())
		width = maxWidth();
	_size.setWidth(width);
    return _size.width();
}

int QVideoStream::setHeight(int height)
{
	if (height < 0)
		height = 0;
	if (height > maxHeight())
		height = maxHeight();
	_size.setHeight(height);
    return _size.height();
}

QSize QVideoStream::inputSize() const
{
	return _inputSize;
}

int QVideoStream::inputWidth() const
{
	return _inputSize.width();
}

int QVideoStream::inputHeight() const
{
	return _inputSize.height();
}

QSize QVideoStream::setInputSize(const QSize& sz)
{
	if (sz == _inputSize)
		return _inputSize;
	_inputSize = sz;
	if (_method & (METHOD_XSHM | METHOD_X11)) {
		deInit();
		init();
	}
#ifdef HAVE_GL
	if (_method & METHOD_GL) {
        d->glwidget->setInputSize(_inputSize);
	}
#endif
    return _inputSize;
}

int QVideoStream::setInputWidth(int width)
{
	if (width == _inputSize.width())
		return _inputSize.width();
	_inputSize.setWidth(width);
	if (_method & (METHOD_XSHM | METHOD_X11)) {
		deInit();
		init();
	}
#ifdef HAVE_GL
	if (_method & METHOD_GL) {
        d->glwidget->setInputSize(_inputSize);
	}
#endif
    return _inputSize.width();
}

int QVideoStream::setInputHeight(int height)
{
	if (height == _inputSize.height())
		return _inputSize.height();
	_inputSize.setHeight(height);
	if (_method & (METHOD_XSHM | METHOD_X11)) {
		deInit();
		init();
	}
#ifdef HAVE_GL
	if (_method & METHOD_GL) {
        d->glwidget->setInputSize(_inputSize);
	}
#endif
    return _inputSize.height();
}

bool QVideoStream::supportsFormat(VideoMethod method, ImageFormat format)
{
    return (bool)(formatsForMethod(method) & format);
}

QVideo::ImageFormat QVideoStream::formatsForMethod(VideoMethod method)
{
    switch(method) {
    case METHOD_XSHM:
    case METHOD_X11:
        return _xFormat;
    case METHOD_XV:
    case METHOD_XVSHM:
        return FORMAT_YUYV;
    case METHOD_GL:
        return FORMAT_BGR24;
    default:
        return FORMAT_NONE;
    }
}

QVideo::ImageFormat QVideoStream::format() const
{
    return _format;
}

bool QVideoStream::setFormat(ImageFormat format)
{
    if(supportsFormat(_method, format)) {
        _format = format;
        return true;
    } else {
        return false;
    }
}

int QVideoStream::displayFrame(const unsigned char *const img)
{
    return displayFrame(img, 0, 0, _inputSize.width(), _inputSize.height());
}

int QVideoStream::displayFrame(const unsigned char *const img, int x, int y, int sw, int sh)
{
	Q_ASSERT(_init);
	if (!_init)
		return -1;

    Q_ASSERT(_methods & _method);
    if (!(_methods & _method))
        return -1;
    
	switch (_method) {
	case METHOD_XV:
	case METHOD_XVSHM:
		return d->xvdev->displayImage(_w, img,
                                      _inputSize.width(), _inputSize.height(), x, y, sw, sh,
                                      _size.width(), _size.height());
        break;
	case METHOD_XSHM:
#ifdef HAVE_XSHM
        memcpy(d->xim->data,img,d->xim->bytes_per_line*d->xim->height);
        XShmPutImage(_w->x11Display(), _w->winId(), d->gc, d->xim,
                     x, y,
                     0, 0,
                     sw, sh,
                     0);
        XSync(_w->x11Display(), False);
        break;
#else
        return -1;
#endif
	case METHOD_X11:
        memcpy(d->xim->data, img, d->xim->bytes_per_line*d->xim->height);
        XPutImage(_w->x11Display(), _w->winId(), d->gc, d->xim,
                  x, y,
                  0, 0,
                  sw, sh);
        XSync(_w->x11Display(), False);
        break;
	case METHOD_GL:
#ifdef HAVE_GL
        d->glwidget->display(img, x, y, sw, sh);
#endif
        break;
	default:
		Q_ASSERT(0);
		return -1;
	}

    return 0;
}

QVideoStream& QVideoStream::operator<<(const unsigned char *const img)
{
	displayFrame(img);
    return *this;
}

// ---------------------------------------------------------------------------------------
#ifdef HAVE_GL

QVideoStreamGLWidget::QVideoStreamGLWidget(QWidget* parent, const char* name)
    : QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::Rgba | QGL::DirectRendering), parent, name),
      _tex(0),
      _w(parent),
      _glfun(false)
{
    kdDebug() << "QVideoStreamGLWidget::QVideoStreamGLWidget()" << endl;

    connect(_w, SIGNAL(resized(int, int)),
            this, SLOT(resize(int, int)));

    topLevelWidget()->installEventFilter(this);
    _glfunTimer = new QTimer();
}

QVideoStreamGLWidget::~QVideoStreamGLWidget()
{
    kdDebug() << "QVideoStreamGLWidget::~QVideoStreamGLWidget()" << endl;
    delete _glfunTimer;

    makeCurrent();
    if(_tex != 0) {
        glDeleteTextures(1, &_tex);
    }
}

bool QVideoStreamGLWidget::eventFilter(QObject*, QEvent* e)
{
    // For some reason, KeyPress does not work (yields 2), QEvent::KeyPress is unknown... What the f...????
    // I am too lazy to scan the header files for the reason.
    if(e->type() == 6) {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);
        if(ke->key() == Qt::Key_Pause) {
            _glfunTimer->start(500, true);
        } else if (_glfunTimer->isActive() && (ke->key() == Qt::Key_Escape)) {
            _glfun = !_glfun;
        }
    }
    return false;
}

void QVideoStreamGLWidget::initializeGL()
{
    kdDebug() << "QVideoStreamGLWidget::initializeGL()" << endl;
    setAutoBufferSwap(false);

    QGLFormat f = format();
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxGL);
    kdDebug() << "OpenGL capabilities (* = required):"           << endl;
    kdDebug() << "    Valid context*:   " << isValid()           << endl;
    kdDebug() << "    DoubleBuffer*:    " << f.doubleBuffer()    << endl;
    kdDebug() << "    Depth:            " << f.depth()           << endl;
    kdDebug() << "    RGBA*:            " << f.rgba()            << endl;
    kdDebug() << "    Alpha:            " << f.alpha()           << endl;
    kdDebug() << "    Accum:            " << f.accum()           << endl;
    kdDebug() << "    Stencil:          " << f.stencil()         << endl;
    kdDebug() << "    Stereo:           " << f.stereo()          << endl;
    kdDebug() << "    DirectRendering*: " << f.directRendering() << endl;
    kdDebug() << "    Overlay:          " << f.hasOverlay()      << endl;
    kdDebug() << "    Plane:            " << f.plane()           << endl;
    kdDebug() << "    MAX_TEXTURE_SIZE: " << _maxGL              << endl;

    qglClearColor(Qt::black);
    glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _vul = QPoint( 4,  10);
    _vur = QPoint(-8,   4);
    _vll = QPoint(10,  -4);
    _vlr = QPoint(-8, -10);
}

void QVideoStreamGLWidget::resizeGL(int w, int h)
{
    _sz = QSize(w, h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    _ul = QPoint(0, 0);
    _ur = QPoint(w, 0);
    _ll = QPoint(0, h);
    _lr = QPoint(w, h);
}

void QVideoStreamGLWidget::setInputSize(const QSize& sz)
{
    makeCurrent();

    _inputSize = sz;
    int iw = _inputSize.width();
    int ih = _inputSize.height();

    if ( (iw > _maxGL) || (ih > _maxGL) ) {
        kdWarning() << "QVideoStreamGLWidget::setInputSize(): Texture too large! maxGL: " << _maxGL << endl;
        return;
    }

    // textures have power-of-two x,y dimensions
    int i;
    for (i = 0; iw >= (1 << i); i++)
        ;
    _tw = (1 << i);
    for (i = 0; ih >= (1 << i); i++)
        ;
    _th = (1 << i);
    
    // Generate texture
    if(_tex != 0) {
        glDeleteTextures(1, &_tex);
    }
    glGenTextures(1, &_tex);
    glBindTexture(GL_TEXTURE_2D, _tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Blank texture
    char* dummy = new char[_tw*_th*4];
    memset(dummy, 128, _tw*_th*4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _tw, _th, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, dummy);
    delete[] dummy;
}

void QVideoStreamGLWidget::display(const unsigned char *const img, int x, int y, int sw, int sh)
{
    makeCurrent();

    // FIXME: Endianess - also support GL_RGB
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _inputSize.width(), _inputSize.height(),
                    GL_BGR, GL_UNSIGNED_BYTE, img);

    // upper right coords
    float ur_x = (float)(x + sw) / _tw;
    float ur_y = (float)(y + sh) / _th;

    // lower left coords
    float ll_x = (float)(x)      / _tw;
    float ll_y = (float)(y)      / _th;
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glBindTexture(GL_TEXTURE_2D, _tex);
    if (!_glfun) {
        glBegin(GL_QUADS);
        glTexCoord2f(ll_x, ur_y); glVertex2i(0,           0           );
        glTexCoord2f(ll_x, ll_y); glVertex2i(0,           _sz.height());
        glTexCoord2f(ur_x, ll_y); glVertex2i(_sz.width(), _sz.height());
        glTexCoord2f(ur_x, ur_y); glVertex2i(_sz.width(), 0           );
        glEnd();
    } else {
        calc(_ul, _vul);
        calc(_ur, _vur);
        calc(_ll, _vll);
        calc(_lr, _vlr);

        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_QUADS);
        glTexCoord2f(0, y); glVertex2i(_ul.x(), _ul.y());
        glTexCoord2f(0, 0); glVertex2i(_ll.x(), _ll.y());
        glTexCoord2f(x, 0); glVertex2i(_lr.x(), _lr.y());
        glTexCoord2f(x, y); glVertex2i(_ur.x(), _ur.y());
        glEnd();
    }
    swapBuffers();
    glDisable(GL_TEXTURE_2D);
}

void QVideoStreamGLWidget::calc(QPoint& p, QPoint& v)
{
    p += v;

    if(p.x() < 0) {
        p.setX(-p.x());
        v.setX(-v.x());
    }
    if(p.y() < 0) {
        p.setY(-p.y());
        v.setY(-v.y());
    }
    if(p.x() > _sz.width()) {
        p.setX(_sz.width() - (p.x() - _sz.width()));
        v.setX(-v.x());
    }
    if(p.y() > _sz.height()) {
        p.setY(_sz.height() - (p.y() - _sz.height()));
        v.setY(-v.y());
    }
} 
#endif

#include "qvideostream.moc"
