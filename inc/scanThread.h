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


using namespace std::chrono;
class ScanThread    :   public QObject// : public QThread
{

    Q_OBJECT
    public:
        ScanThread(QObject *parent = nullptr);
        ~ScanThread();

        bool setFolderPath(const string &path);

        void run();
        void cancelScan();

        bool isRunning();
        double getRuntime();



        const Folder &getFolderResult() const;

        // FolderInterface
        size_t getMasterFolderCount();
        size_t getMasterFileCount();
        uintmax_t getMasterContentSize();

        void lockThread();
        void unlockThread();


signals:
    void resultReady(ScanThread *thread,const Folder &);

private slots:
    void onThreadFinishedUpdateTimerFinished();

    private:
    struct ThreadData
    {
        std::atomic<bool> *done;
        Folder *fp;
        std::atomic<high_resolution_clock::time_point> *endPoint;
    };
    static void threadFunc(ThreadData data);

    Folder *m_folder;
    string m_path;
    //QMutex m_mutex;
    std::mutex m_mutex;

    std::thread *m_thread;
    std::atomic<bool> m_threadDone;
    QTimer *m_threadFinishedUpdateTimer;

    bool m_locked;

    // statistics
    high_resolution_clock::time_point m_startTime;
    std::atomic<high_resolution_clock::time_point> m_endTime;
    duration<double> m_time_span;
};

#endif // SCANTHREAD_H
