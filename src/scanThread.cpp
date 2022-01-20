#include "scanThread.h"

ScanThread::ScanThread(QObject *parent)
    :QObject(parent)//:   QThread(parent)
{
    m_folder = nullptr;
    m_locked = false;
    //this->setTerminationEnabled(false);
    m_thread = nullptr;
    m_threadDone = 1;

    m_threadFinishedUpdateTimer = new QTimer(this);
    connect(m_threadFinishedUpdateTimer,&QTimer::timeout,this,&ScanThread::onThreadFinishedUpdateTimerFinished);

}
ScanThread::~ScanThread()
{
    if(m_folder != nullptr)
        delete m_folder;
    if(m_thread != nullptr)
    {
        m_thread->join();
        delete m_thread;
    }
}
bool ScanThread::setFolderPath(const string &path)
{
    if(!Folder::dirExists(path))
    {
        qDebug() << "path not found";
        QMessageBox::critical(nullptr, tr("File scanner"),
                                    tr("This path is not valid\n"),
                                    QMessageBox::Ok);
        return false;
    }
    m_path = path;
    return true;
}
void ScanThread::reset()
{
    if(!m_threadDone)
        return;

    if(m_folder != nullptr)
        delete m_folder;
    m_folder = new Folder;
    m_folder->setName(m_path);
}
void ScanThread::runScan()
{
    if(!m_threadDone)
        return;

    if(m_thread != nullptr)
    {
        m_thread->join();
        delete m_thread;
    }
    if(!m_folder)
        reset();

    //qDebug() << "Thread begin";

    m_startTime = high_resolution_clock::now();
    m_endTime   = m_startTime;



    ThreadData data;
    data.fp = m_folder;
    data.done = &m_threadDone;
    data.endPoint = &m_endTime;

    m_threadFinishedUpdateTimer->start(100);
    m_threadDone = false;
    m_thread = new std::thread(threadScanFunc,data);
  //  qDebug()<< "Thread is started";

    //emit resultReady(this,*m_folder);
    //qDebug() << "Thread end";
}
void ScanThread::runMD5Scan()
{
    if(!m_threadDone)
        return;

    if(m_thread != nullptr)
    {
        m_thread->join();
        delete m_thread;
    }

    //qDebug() << "Thread begin";

    m_startTime = high_resolution_clock::now();
    m_endTime   = m_startTime;


    ThreadData data;
    data.fp = m_folder;
    data.done = &m_threadDone;
    data.endPoint = &m_endTime;

    m_threadFinishedUpdateTimer->start(100);
    m_threadDone = false;
    m_thread = new std::thread(threadScanMD5Func,data);
}
void ScanThread::cancelScan()
{
    bool lk = m_locked;

    if(!lk)
        lockThread();

    m_folder->cancelScan();

    if(!lk)
        unlockThread();
}

bool ScanThread::isRunning()
{
    bool val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = !m_threadDone;

    if(!lk)
        unlockThread();
    return val;
}
double ScanThread::getRuntime()
{
    if(isRunning())
    {
        high_resolution_clock::time_point timePoint = high_resolution_clock::now();
        m_time_span = duration_cast<duration<double>>(timePoint - m_startTime);
        return m_time_span.count();
    }
    high_resolution_clock::time_point timePoint = m_endTime;
    m_time_span = duration_cast<duration<double>>(timePoint - m_startTime);
    return m_time_span.count();
}

const Folder &ScanThread::getFolderResult() const
{
    return *m_folder;
}

size_t ScanThread::getMasterFolderCount()
{
    size_t val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = m_folder->getMasterFolderCount();

    if(!lk)
        unlockThread();
    return val;
}
size_t ScanThread::getMasterFileCount()
{
    size_t val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = m_folder->getMasterFileCount();

    if(!lk)
        unlockThread();
    return val;
}
uintmax_t ScanThread::getMasterContentSize()
{
    uintmax_t val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = m_folder->getMasterContentSize();

    if(!lk)
        unlockThread();
    return val;
}
double ScanThread::getMasterMD5ScanProgress()
{
    double val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = m_folder->getMasterMD5ScanProgress();

    if(!lk)
        unlockThread();
    return val;
}

void ScanThread::lockThread()
{
    if(!m_locked)
        m_mutex.lock();
    m_locked = true;
}
void ScanThread::unlockThread()
{
    if(m_locked)
        m_mutex.unlock();
    m_locked = false;
}

void ScanThread::threadScanFunc(ThreadData data)
{
   // qDebug()<< "ThreadFunc";
    data.fp->scan();
    *data.endPoint = high_resolution_clock::now();
    *data.done = true;
}
void ScanThread::threadScanMD5Func(ThreadData data)
{
   // qDebug()<< "ThreadFunc";
    data.fp->scanMd5();
    *data.endPoint = high_resolution_clock::now();
    *data.done = true;
}
void ScanThread::onThreadFinishedUpdateTimerFinished()
{
    if(!isRunning())
    {
        m_threadFinishedUpdateTimer->stop();
       // qDebug() << "Thread finished";
        emit resultReady(this,*m_folder);
    }
    else
    {
     //   qDebug() << "Thread is running";
    }
}
