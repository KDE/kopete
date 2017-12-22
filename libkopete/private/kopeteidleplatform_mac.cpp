/*
    kopeteidleplatform_mac.cpp  -  Kopete Idle Platform

    Copyright (C) 2003      by Tarkvara Design Inc.  (from KVIrc source code)
    Copyright (c) 2008      by Roman Jarosz          <kedgedev@centrum.cz>
    Kopete    (c) 2008      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteidleplatform_p.h"
#include <Carbon/Carbon.h>

// Why does Apple have to make this so complicated?
static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
    OSStatus err;
    FSRef frameworksFolderRef;
    CFURLRef baseURL;
    CFURLRef bundleURL;

    if (bundlePtr == nil) {
        return -1;
    }

    *bundlePtr = nil;

    baseURL = nil;
    bundleURL = nil;

    err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
    if (err == noErr) {
        baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
        if (baseURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }

    if (err == noErr) {
        bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
        if (bundleURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }

    if (err == noErr) {
        *bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
        if (*bundlePtr == nil) {
            err = coreFoundationUnknownErr;
        }
    }

    if (err == noErr) {
        if (!CFBundleLoadExecutable(*bundlePtr)) {
            err = coreFoundationUnknownErr;
        }
    }

    // Clean up.
    if (err != noErr && *bundlePtr != nil) {
        CFRelease(*bundlePtr);
        *bundlePtr = nil;
    }

    if (bundleURL != nil) {
        CFRelease(bundleURL);
    }

    if (baseURL != nil) {
        CFRelease(baseURL);
    }

    return err;
}

class Kopete::IdlePlatform::Private
{
public:
    EventLoopTimerRef mTimerRef;
    int mSecondsIdle;

    Private() : mTimerRef(0)
        , mSecondsIdle(0)
    {
    }

    static pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void *inUserData);
};

pascal void Kopete::IdlePlatform::Private::IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void *inUserData)
{
    switch (inState) {
    case kEventLoopIdleTimerStarted:
    case kEventLoopIdleTimerStopped:
        // Get invoked with this constant at the start of the idle period,
        // or whenever user activity cancels the idle.
        ((Kopete::IdlePlatform::Private *)inUserData)->mSecondsIdle = 0;
        break;
    case kEventLoopIdleTimerIdling:
        // Called every time the timer fires (i.e. every second).
        ((Kopete::IdlePlatform::Private *)inUserData)->mSecondsIdle++;
        break;
    }
}

Kopete::IdlePlatform::IdlePlatform()
    : d(new Private())
{
}

Kopete::IdlePlatform::~IdlePlatform()
{
    RemoveEventLoopTimer(d->mTimerRef);
    delete d;
}

// Typedef for the function we're getting back from CFBundleGetFunctionPointerForName.
typedef OSStatus (*InstallEventLoopIdleTimerPtr)(EventLoopRef inEventLoop, EventTimerInterval inFireDelay, EventTimerInterval inInterval, EventLoopIdleTimerUPP inTimerProc, void *inTimerData,
                                                 EventLoopTimerRef *outTimer);

bool Kopete::IdlePlatform::init()
{
    // May already be init'ed.
    if (d->mTimerRef) {
        return true;
    }

    // According to the docs, InstallEventLoopIdleTimer is new in 10.2.
    // According to the headers, it has been around since 10.0.
    // One of them is lying.  We'll play it safe and weak-link the function.

    // Load the "Carbon.framework" bundle.
    CFBundleRef carbonBundle;
    if (LoadFrameworkBundle(CFSTR("Carbon.framework"), &carbonBundle) != noErr) {
        return false;
    }

    // Load the Mach-O function pointers for the routine we will be using.
    InstallEventLoopIdleTimerPtr myInstallEventLoopIdleTimer = (InstallEventLoopIdleTimerPtr)CFBundleGetFunctionPointerForName(carbonBundle, CFSTR("InstallEventLoopIdleTimer"));
    if (myInstallEventLoopIdleTimer == 0) {
        return false;
    }

    EventLoopIdleTimerUPP timerUPP = NewEventLoopIdleTimerUPP(Private::IdleTimerAction);
    if ((*myInstallEventLoopIdleTimer)(GetMainEventLoop(), kEventDurationSecond, kEventDurationSecond, timerUPP, 0, &d->mTimerRef)) {
        return true;
    }

    return false;
}

int Kopete::IdlePlatform::secondsIdle()
{
    return d->mSecondsIdle;
}
