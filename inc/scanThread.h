#ifndef SCANTHREAD_H
#define SCANTHREAD_H

//#include <QThread>
#include <QMessageBox>
//#include <QMutex>

#include <thread>         // std::thread
#include <mutex>
#include <atomic>
#include <QTimer>
#include <ctime>
#include <ratio>
#include <chrono>
#include "folder.h"
#include "utility.h"

enum Task
{
    none,
    fileScan,
    md5Scan,
    lineCounter
};
using namespace std::chrono;
class ScanThread    :   public QObject// : public QThread
{

    Q_OBJECT
    public:
        ScanThread(QObject *parent = nullptr);
        ~ScanThread();

        bool setFolderPath(const string &path);

        void reset();
        void runScan();
        void runMD5Scan();
        void runLineCounter();
        void cancelScan();

        bool isRunning();
        Task getCurrentTask();
        double getRuntime();




        Folder *getFolderResult() const;

        // FolderInterface
        size_t getMasterFolderCount();
        size_t getMasterFileCount();
        uintmax_t getMasterContentSize();
        double getMasterProgress();

        void lockThread();
        void unlockThread();


signals:
    void onReset();
    void resultReady(ScanThread *thread,const Folder &);

private slots:
    //void onThreadFinishedUpdateTimerFinished();

    private:
    struct ThreadDataScan
    {
        std::atomic<bool> *done;
        Folder* fp;
        //std::atomic<high_resolution_clock::time_point> *endPoint;
    };
    struct ThreadData
    {
        size_t threadIndex;
        Folder *masterFolder;
        File **list;
        size_t listSize;
        size_t *beginPoint;
        size_t jobSize;
        bool *exit;
        int *threadsActiveCount;
    };


    static void threadScanFunc(ThreadDataScan data);
    static void fastThreadScanMD5Func(ThreadData *data);
    static void fastThreadCountLinesFunc(ThreadData *data);

    Folder *m_folder;
    string m_path;
    static std::mutex m_mutex;

    // file scanner
    std::thread *m_scanThread;
    std::atomic<bool> m_threadDone;

    // md5 scanner


    // line counter




    bool m_locked;
    Task m_currentTask;
    bool m_exitTask;


    // statistics
    high_resolution_clock::time_point m_startTime;
    std::atomic<high_resolution_clock::time_point> m_endTime;
    duration<double> m_time_span;
};

#endif // SCANTHREAD_H
