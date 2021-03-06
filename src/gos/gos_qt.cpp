/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.io/license.html
 */

#include "../../gfx.h"

#if GFX_USE_OS_QT
#include <QMutex>
#include <QSemaphore>
#include <QThread>
#include <QElapsedTimer>

extern "C" void _gosPostInit(void);

class Thread : public QThread
{
public:
    typedef gThreadreturn (*fptr)(void* param);

    void setFunction(fptr function, void* param)
    {
        _function = function;
        _param = param;
    }

    gThreadreturn returnValue()
    {
        return _returnValue;
    }

    virtual void run() override
    {
        if (!_function) {
            return;
        }

        _returnValue = _function(_param);
    }

private:
    fptr _function;
    void* _param;
    gThreadreturn _returnValue;
};

static QElapsedTimer _systickTimer;
static QMutex _systemMutex;

void _gosInit(void)
{
    _systickTimer.start();
}

void _gosPostInit(void)
{
}

void _gosDeinit(void)
{
}

void gfxHalt(const char *msg)
{
    volatile uint32_t dummy;

    (void)msg;

    while(1) {
        dummy++;
    }
}

void gfxExit(void)
{
    volatile uint32_t dummy;

    while(1) {
        dummy++;
    }
}

void* gfxAlloc(size_t sz)
{
    return malloc(sz);
}

void* gfxRealloc(void* ptr, size_t oldsz, size_t newsz)
{
    Q_UNUSED(oldsz)
    return realloc(ptr, newsz);
}

void gfxFree(void* ptr)
{
    free(ptr);
}

void gfxYield(void)
{
    QThread::msleep(0);
}

void gfxSleepMilliseconds(gDelay ms)
{
    QThread::msleep(ms);
}

void gfxSleepMicroseconds(gDelay us)
{
    QThread::usleep(us);
}

gTicks gfxSystemTicks(void)
{
    return _systickTimer.elapsed();
}

gTicks gfxMillisecondsToTicks(gDelay ms)
{
    return ms;
}

void gfxSystemLock(void)
{
    _systemMutex.lock();
}

void gfxSystemUnlock(void)
{
    _systemMutex.unlock();
}

void gfxMutexInit(gfxMutex *pmutex)
{
    *pmutex = new QMutex;
}

void gfxMutexDestroy(gfxMutex *pmutex)
{
    delete static_cast<QMutex*>(*pmutex);
}

void gfxMutexEnter(gfxMutex *pmutex)
{
    static_cast<QMutex*>(*pmutex)->lock();
}

void gfxMutexExit(gfxMutex *pmutex)
{
    static_cast<QMutex*>(*pmutex)->unlock();
}

void gfxSemInit(gfxSem *psem, gSemcount val, gSemcount limit)
{
    *psem = new QSemaphore(limit);

    static_cast<QSemaphore*>(*psem)->release(val);
}

void gfxSemDestroy(gfxSem *psem)
{
    delete static_cast<QSemaphore*>(*psem);
}

gBool gfxSemWait(gfxSem *psem, gDelay ms)
{
    return static_cast<QSemaphore*>(*psem)->tryAcquire(1, ms);
}

gBool gfxSemWaitI(gfxSem *psem)
{
    return static_cast<QSemaphore*>(*psem)->tryAcquire(1);
}

void gfxSemSignal(gfxSem *psem)
{
    static_cast<QSemaphore*>(*psem)->release(1);
}

void gfxSemSignalI(gfxSem *psem)
{
    static_cast<QSemaphore*>(*psem)->release(1);
}

gThread gfxThreadCreate(void *stackarea, size_t stacksz, gThreadpriority prio, DECLARE_THREAD_FUNCTION((*fn),p), void *param)
{
    Q_UNUSED(stackarea)

    Thread* thread = new Thread;
    thread->setFunction(fn, param);
    if (stacksz > 0) {
        thread->setStackSize(stacksz);
    }
    thread->start(static_cast<QThread::Priority>(prio));

    return static_cast<gThread>(thread);
}

gThreadreturn gfxThreadWait(gThread thread)
{
    Thread* t = static_cast<Thread*>(thread);

    gThreadreturn returnValue = t->returnValue();
    t->wait();
    t->exit();

    return returnValue;
}

gThread gfxThreadMe(void)
{
    return static_cast<Thread*>(QThread::currentThread());
}

void gfxThreadClose(gThread thread)
{
    static_cast<Thread*>(thread)->exit();
}

#endif /* GFX_USE_OS_QT */
