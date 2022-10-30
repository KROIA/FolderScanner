#include "scanThread.h"

std::mutex ScanThread::m_mutex;
ScanThread::ScanThread(QObject *parent)
    :QObject(parent)//:   QThread(parent)
{
    m_folder = nullptr;
    m_locked = false;
    //this->setTerminationEnabled(false);
    m_scanThread = nullptr;
    m_threadDone = 1;
    m_exitTask = false;
    m_currentTask = Task::none;

    //m_threadFinishedUpdateTimer = new QTimer(this);
    //connect(m_threadFinishedUpdateTimer,&QTimer::timeout,this,&ScanThread::onThreadFinishedUpdateTimerFinished);

}
ScanThread::~ScanThread()
{
    if(m_folder != nullptr)
        delete m_folder;
    if(m_scanThread != nullptr)
    {
        m_scanThread->join();
        delete m_scanThread;
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
    if(m_currentTask)
        return;

    if(m_folder != nullptr)
        delete m_folder;
    m_folder = new Folder;
    m_folder->setName(m_path);
    emit onReset();
}
void ScanThread::runScan()
{
    if(m_currentTask)
        return;



    if(!m_folder)
        reset();

    qDebug() << "ScanThread::runScan() begin";

    m_startTime = high_resolution_clock::now();
    m_endTime   = m_startTime;


    m_currentTask = Task::fileScan;
    ThreadDataScan data;
    data.fp = m_folder;
    data.done = &m_threadDone;
    //data.endPoint = &m_endTime;

    //m_threadFinishedUpdateTimer->start(100);
    m_threadDone = false;
    m_scanThread = new std::thread(threadScanFunc,data);

    bool threadFinished = false;
    while(!threadFinished)
    {
        if(processQEvents())
        {
            {
                std::unique_lock<std::mutex> lck (m_mutex);
                threadFinished = m_threadDone;
            }
            //if(!threadFinished)
            //    qDebug() << "waiting for thread to finish";
        }
    }
    m_scanThread->join();
    delete m_scanThread;
    m_scanThread = nullptr;
    m_currentTask = Task::none;
    m_endTime   = high_resolution_clock::now();

    // qDebug() << "Thread finished";
    emit resultReady(this,*m_folder);
    qDebug() << "ScanThread::runScan() end";

  //  qDebug()<< "Thread is started";

    //emit resultReady(this,*m_folder);
    //qDebug() << "Thread end";
}
void ScanThread::runMD5Scan()
{

//#define DEBUG_FAST_HASH_THREAD
    if(m_currentTask || !m_folder)
        return;


    m_startTime = high_resolution_clock::now();
    m_currentTask = Task::md5Scan;
    size_t numThreads = std::thread::hardware_concurrency();
    vector<std::thread* > threadList;
    vector<ThreadData*> threadData;
    vector<File*> files;
    //vector<Folder*> folders;
    m_folder->getAllFilesRecursive(files);
    size_t fileCount = files.size();
    size_t jobSize = fileCount/(10*numThreads)+1;

    size_t threadData_beginPoint = 0;
    //bool threadData_exit = false;
    m_exitTask = false;
    int threadData_threadsActiveCount = 0;

    threadList.resize(numThreads,nullptr);
    threadData.resize(numThreads,nullptr);

    m_folder->masterResetProgress();
    for(size_t i=0; i<numThreads; ++i)
    {
        threadData[i] = new ThreadData{
                .threadIndex=i,
                .masterFolder=m_folder,
                .list=files.data(),
                .listSize=fileCount,
                .beginPoint=&threadData_beginPoint,
                .jobSize=jobSize,
                .exit=&m_exitTask,
                .threadsActiveCount=&threadData_threadsActiveCount};
    }
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "exec threads";
#endif
    for(size_t i=0; i<numThreads; ++i)
    {
        threadList[i] = new std::thread(fastThreadScanMD5Func,threadData[i]);
    }
    //m_threadFinishedUpdateTimer->start(100);
    //m_folder->getAllFoldersRecursive(folders);
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "waiting for join";
#endif
    bool allFinished = false;
    int activeThreads = numThreads;
    using namespace std::chrono;
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();
    high_resolution_clock::time_point t3 = high_resolution_clock::now();

    do{
        //high_resolution_clock::time_point t2 = high_resolution_clock::now();
        high_resolution_clock::time_point t4 = high_resolution_clock::now();
        //duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        duration<double> time_span2 = duration_cast<duration<double>>(t4 - t3);

        if(processQEvents())
        {
#ifdef DEBUG_FAST_HASH_THREAD
            qDebug() << "still waiting for threads to finish. Running: "<<activeThreads;
#endif
        }
        if(time_span2.count() > 0.01)
        {
            t3 = t4;
            {
                std::unique_lock<std::mutex> lck (m_mutex);
                activeThreads = threadData_threadsActiveCount;
            }
            if(activeThreads == 0)
                allFinished = true;
        }
    }while(!allFinished);

    for(size_t i=0; i<numThreads; ++i)
    {
        threadList[i]->join();
        delete threadList[i];
        delete threadData[i];
        threadList.clear();
        threadData.clear();
    }
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "all threads joined";
    qDebug() << "hashing folders";
#endif
    /*for(size_t i=0; i<folders.size(); ++i)
    {
        Folder::md5(*folders[i]);
    }*/
    if(!m_exitTask)
        Folder::md5(*m_folder);
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "hashing finished";
#endif

    m_currentTask = Task::none;

    m_endTime = high_resolution_clock::now();
    emit resultReady(this,*m_folder);


}
void ScanThread::runLineCounter()
{
    if(m_currentTask || !m_folder)
        return;

    m_startTime = high_resolution_clock::now();
    m_currentTask = Task::lineCounter;
    size_t numThreads = std::thread::hardware_concurrency();
    vector<std::thread* > threadList;
    vector<ThreadData*> threadData;
    vector<File*> files;

    m_folder->getAllFilesRecursive(files);
    size_t fileCount = files.size();
    size_t jobSize = fileCount/(10*numThreads)+1;

    size_t threadData_beginPoint = 0;

    m_exitTask = false;
    int threadData_threadsActiveCount = 0;

    threadList.resize(numThreads,nullptr);
    threadData.resize(numThreads,nullptr);

    m_folder->masterResetProgress();
    for(size_t i=0; i<numThreads; ++i)
    {
        threadData[i] = new ThreadData{
                .threadIndex=i,
                .masterFolder=m_folder,
                .list=files.data(),
                .listSize=fileCount,
                .beginPoint=&threadData_beginPoint,
                .jobSize=jobSize,
                .exit=&m_exitTask,
                .threadsActiveCount=&threadData_threadsActiveCount};
    }
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "exec threads";
#endif
    for(size_t i=0; i<numThreads; ++i)
    {
        threadList[i] = new std::thread(fastThreadCountLinesFunc,threadData[i]);
    }

#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "waiting for join";
#endif
    bool allFinished = false;
    int activeThreads = numThreads;
    using namespace std::chrono;
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();
    high_resolution_clock::time_point t3 = high_resolution_clock::now();

    do{
        //high_resolution_clock::time_point t2 = high_resolution_clock::now();
        high_resolution_clock::time_point t4 = high_resolution_clock::now();
        //duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        duration<double> time_span2 = duration_cast<duration<double>>(t4 - t3);

        if(processQEvents())
        {
#ifdef DEBUG_FAST_HASH_THREAD
            qDebug() << "still waiting for threads to finish. Running: "<<activeThreads;
#endif
        }
        if(time_span2.count() > 0.01)
        {
            t3 = t4;
            {
                std::unique_lock<std::mutex> lck (m_mutex);
                activeThreads = threadData_threadsActiveCount;
            }
            if(activeThreads == 0)
                allFinished = true;
        }
    }while(!allFinished);

    for(size_t i=0; i<numThreads; ++i)
    {
        threadList[i]->join();
        delete threadList[i];
        delete threadData[i];
        threadList.clear();
        threadData.clear();
    }
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "all threads joined";
#endif


    m_currentTask = Task::none;

    m_endTime = high_resolution_clock::now();
    emit resultReady(this,*m_folder);
}
void ScanThread::cancelScan()
{

    if(m_currentTask == Task::md5Scan)
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        m_exitTask = true;
    }

    if(!m_folder)
        return;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    m_folder->cancelScan();

    if(!lk)
        unlockThread();
}

bool ScanThread::isRunning()
{

    if(!m_folder || !m_currentTask)
    {
        //qDebug()<<"not bussy";
        return false;
    }
    return m_currentTask;
  //  bool val = false;
  //  if(m_currentTask == Task::md5Scan ||
  //     m_currentTask == Task::lineCounter)
  //  {
  //       return true;
  //  }


  //  bool lk = m_locked;

  //  if(!lk)
  //      lockThread();

  //  if(m_currentTask == Task::fileScan)
  //  {
  //      val = !m_threadDone;
  //  }

  //  if(!lk)
  //      unlockThread();


  //  return val;
}
Task ScanThread::getCurrentTask()
{
    return m_currentTask;
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

Folder *ScanThread::getFolderResult() const
{
    return m_folder;
}

size_t ScanThread::getMasterFolderCount()
{
    size_t val = 0;
    if(!m_folder)
        return val;
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
    size_t val = 0;
    if(!m_folder)
        return val;
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
    uintmax_t val = 0;
    if(!m_folder)
        return val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

    val = m_folder->getMasterContentSize();
    if(!lk)
        unlockThread();
    return val;
}
double ScanThread::getMasterProgress()
{
    double val = 0;
    if(!m_folder)
        return val;
    bool lk = m_locked;

    if(!lk)
        lockThread();

     val = m_folder->getMasterProgress();


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

void ScanThread::threadScanFunc(ThreadDataScan data)
{
    // qDebug()<< "ThreadFunc";
    //data.fp->scan();
    data.fp->scanThreaded();
    //auto t  = high_resolution_clock::now();

    {
        std::unique_lock<std::mutex> lck (m_mutex);
        //*data.endPoint = t;
        *data.done = true;
    }
}

void ScanThread::fastThreadScanMD5Func(ThreadData *data)
{

    if(!data)
        return;
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "starte Thread: "<<data->threadIndex;
    size_t fileProcessedCount = 0;
#endif

    static std::mutex mtx;
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        ++*data->threadsActiveCount;
    }
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    bool exit = false;
    while(1)
    {

        File **fileBegin = nullptr;
        size_t jobSize = data->jobSize;
        size_t currentBeginIndex = 0;

        {
            std::unique_lock<std::mutex> lck (mtx);
            if(*data->beginPoint >= data->listSize)
                goto exitPoint;
            currentBeginIndex = *data->beginPoint;
            fileBegin = data->list;

            *data->beginPoint += jobSize;

        }
        {
            std::unique_lock<std::mutex> lck (m_mutex);
            exit = *data->exit;
        }
        if(exit)
            goto exitPoint;
        fileBegin += currentBeginIndex;
        if(data->listSize < currentBeginIndex+jobSize)
            jobSize = data->listSize - currentBeginIndex;
#ifdef DEBUG_FAST_HASH_THREAD
        qDebug() << "T "<<data->threadIndex << " Hashe index "<<currentBeginIndex << " - " <<currentBeginIndex+jobSize <<" of " << data->listSize;
#endif
        uintmax_t progress = 0;
        for(size_t i=0; i<jobSize; ++i)
        {
            File::md5(*fileBegin[i]);
#ifdef DEBUG_FAST_HASH_THREAD
            ++fileProcessedCount;
#endif
            progress += fileBegin[i]->getSize();
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            if(time_span.count() > 1)
            {
                t1 = t2;
                {
                    std::unique_lock<std::mutex> lck (m_mutex);
                    exit = *data->exit;
                }
                if(exit)
                    goto exitPoint;
                data->masterFolder->masterAddProgress(progress);
                progress = 0;
            }
        }
        data->masterFolder->masterAddProgress(progress);
    }

    exitPoint:


#ifdef DEBUG_FAST_HASH_THREAD
    if(exit)
        qDebug() << "Thread: "<<data->threadIndex<<" abgebrochen. "<< " Files hashed: "<<fileProcessedCount;
    else
        qDebug() << "Thread: "<<data->threadIndex<<" wird beendet. Files hashed: "<<fileProcessedCount;
#endif
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        --*data->threadsActiveCount;
    }
}
void ScanThread::fastThreadCountLinesFunc(ThreadData *data)
{

    if(!data)
        return;
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "starte Thread: "<<data->threadIndex;
    size_t fileProcessedCount = 0;
#endif

    static std::mutex mtx;
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        ++*data->threadsActiveCount;
    }
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    bool exit = false;
    while(1)
    {

        File **fileBegin = nullptr;
        size_t jobSize = data->jobSize;
        size_t currentBeginIndex = 0;

        {
            std::unique_lock<std::mutex> lck (mtx);
            if(*data->beginPoint >= data->listSize)
                goto exitPoint;
            currentBeginIndex = *data->beginPoint;
            fileBegin = data->list;

            *data->beginPoint += jobSize;

        }
        {
            std::unique_lock<std::mutex> lck (m_mutex);
            exit = *data->exit;
        }
        if(exit)
            goto exitPoint;
        fileBegin += currentBeginIndex;
        if(data->listSize < currentBeginIndex+jobSize)
            jobSize = data->listSize - currentBeginIndex;
#ifdef DEBUG_FAST_HASH_THREAD
        qDebug() << "T "<<data->threadIndex << " Hashe index "<<currentBeginIndex << " - " <<currentBeginIndex+jobSize <<" of " << data->listSize;
#endif
        uintmax_t progress = 0;
        for(size_t i=0; i<jobSize; ++i)
        {
            File::countLines(*fileBegin[i]);
#ifdef DEBUG_FAST_HASH_THREAD
            ++fileProcessedCount;
#endif
            progress += fileBegin[i]->getSize();
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            if(time_span.count() > 1)
            {
                t1 = t2;
                {
                    std::unique_lock<std::mutex> lck (m_mutex);
                    exit = *data->exit;
                }
                if(exit)
                    goto exitPoint;
            }
        }
        data->masterFolder->masterAddProgress(progress);
    }

    exitPoint:


#ifdef DEBUG_FAST_HASH_THREAD
    if(exit)
        qDebug() << "Thread: "<<data->threadIndex<<" abgebrochen. "<< " Files hashed: "<<fileProcessedCount;
    else
        qDebug() << "Thread: "<<data->threadIndex<<" wird beendet. Files hashed: "<<fileProcessedCount;
#endif
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        --*data->threadsActiveCount;
    }
}
