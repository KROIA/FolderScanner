#include "fileScanPage.h"
#include "ui_fileScanPage.h"

FileScanPage::FileScanPage(ScanThread* scanThread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileScanPage)
{
    ui->setupUi(this);
    m_treeView = new TreeViewPage(ui->displayFrame);
    m_scanThread = scanThread;
    connect(m_scanThread,&ScanThread::onReset,this,&FileScanPage::clear);
    m_updateTimer = new QTimer;
    connect(m_updateTimer,&QTimer::timeout,this,&FileScanPage::onUpdateTimer);
}

FileScanPage::~FileScanPage()
{
    delete m_treeView;
    delete ui;
}

void FileScanPage::scanFiles()
{
    if(m_scanThread->isRunning())
        return;
    emit setProgress(0,"Scanning directory...");
    m_updateTimer->start(500);

    m_scanThread->runScan();

    m_updateTimer->stop();
    emit setProgress(1,"");
}
void FileScanPage::scanMD5()
{
    if(m_scanThread->isRunning())
        return;
    Folder *f = m_scanThread->getFolderResult();
    if(!f)
        return;

    emit setProgress(0);
    m_updateTimer->start(500);

    m_scanThread->runMD5Scan();

    m_updateTimer->stop();
    emit setProgress(1);
}

void FileScanPage::updateTree()
{

    Folder *f = m_scanThread->getFolderResult();
    if(!f)
        return;
    vector<string> header = {"Name","Size","Grösse [bytes]","MD5 Hash"};
    m_treeView->setTree(f->getStandardItem(),
                        header);
    size_t folderCount;
    size_t fileCount;
    uintmax_t contentSize;
    double runtime;

    m_scanThread->lockThread();
    folderCount = m_scanThread->getMasterFolderCount();
    fileCount   = m_scanThread->getMasterFileCount();
    contentSize = m_scanThread->getMasterContentSize();
    runtime = m_scanThread->getRuntime();
    m_scanThread->unlockThread();


    qDebug() << "update: Folders: "<<folderCount<<"\tFiles: "<<fileCount<<"\tSize: "<<FileData::sizeToStr(contentSize).c_str()<<"\tTime: "<<runtime;
    ui->foldersLabel->setText(QString::number(folderCount));
    ui->filesLabel->setText(QString::number(fileCount));
    ui->timeLabel->setText(getTimeString(runtime).c_str());
    ui->sizeLabel->setText(FileData::sizeToStr(contentSize).c_str());
}
void FileScanPage::clear()
{
    m_treeView->clear();
}
void FileScanPage::onUpdateTimer()
{
    switch(m_scanThread->getCurrentTask())
    {
        case Task::fileScan:
        {
            updateTree();
            break;
        }
        case Task::md5Scan:
        {
            string text = "Creating MD5 hash for each file...";
            static double timePrediction = 1;
            double progress = m_scanThread->getMasterProgress();
            double neededProgress = 1-progress;

            double runtime = m_scanThread->getRuntime();
            timePrediction = runtime*neededProgress/progress;


            qDebug() << "Scan: "<<progress*100<<"%\t"<<timePrediction<<"s";

            if(progress > 0)
            {
                //ui->scanTimePrediction_label->setText(getTimeString(timePrediction).c_str());
                text += "\n"+getTimeString(timePrediction);
                emit setProgress(progress,text);
            }
            else
                emit setProgress(0.01,text);
            updateTree();
            break;
        }
        default:
        {}
    }
}
