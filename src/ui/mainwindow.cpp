#include "mainwindow.h"
#include "ui_mainwindow.h"


const string MainWindow::m_versionsText = "© by Alex Krieg\n18.04.2022\nv00.00.07";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_scanNeedsUpdate = false;
    m_md5NeedsUpdate  = false;


    // Hide ribbon dock title bar
    ui->ribbonDockWidget->setTitleBarWidget(new QWidget());
    m_ribbon = new MainWindowRibbon(ui->ribbonTabWidget);




    scanThread = new ScanThread();
    connect(scanThread, &ScanThread::resultReady, this, &MainWindow::onScanThreadResultFinish);

    m_fileScanTreeView = new FileScanPage(scanThread);
    m_compareScanPage = new CompareScanPage;
    m_duplicateScanTreeView = new DuplicateScanPage(scanThread);
    m_lineCountPage = new LineCountPage(scanThread);

    connect(m_compareScanPage,&CompareScanPage::setProgress,this,&MainWindow::setProgress);
    connect(m_duplicateScanTreeView,&DuplicateScanPage::setProgress,this,&MainWindow::setProgress);
    connect(m_fileScanTreeView,&FileScanPage::setProgress,this,&MainWindow::setProgress);
    connect(m_lineCountPage,&LineCountPage::setProgress,this,&MainWindow::setProgress);

    ui->tabWidget->addTab(m_fileScanTreeView,"Directory page");
    ui->tabWidget->addTab(m_compareScanPage,"Compare page");
    ui->tabWidget->addTab(m_duplicateScanTreeView,"Duplicate page");
    ui->tabWidget->addTab(m_lineCountPage,"File content stats");

   // threadTaskUpdateTimer = new QTimer(this);
   // connect(threadTaskUpdateTimer,&QTimer::timeout,this,&MainWindow::onThreadTaskUpdateTimer);
    //scanMd5UpdateTimer = new QTimer(this);
    //connect(scanMd5UpdateTimer,&QTimer::timeout,this,&MainWindow::onScanMd5UpdateTimer);

    connect(ui->actionExport_button,&QAction::triggered,this,&MainWindow::onExportButtonClicked);
    connect(ui->actionCompare_button,&QAction::triggered,this,&MainWindow::onCompareButtonClicked);
    connect(ui->actionVersion,&QAction::triggered,this,&MainWindow::onVersionButtonClicked);

    // Setup ribbon events
    ProjectButtons prjB = m_ribbon->getProjectButtons();
    connect(prjB.loadPathButton,&QToolButton::clicked,this,&MainWindow::onLoadPath);
    connect(prjB.saveResultButton,&QToolButton::clicked,this,&MainWindow::onExportButtonClicked);
    connect(prjB.scanFolderButton,&QToolButton::clicked,this,&MainWindow::on_rescan_pushButton_clicked);
    connect(prjB.scanMd5Button,&QToolButton::clicked,this,&MainWindow::on_scanMd5_pushButton_clicked);
    connect(prjB.cancelScanButton,&QToolButton::clicked,this,&MainWindow::on_cancel_pushButton_clicked);

    ToolButtons toolB = m_ribbon->getToolButtons();
    //connect(toolB.compareLoadFirstFileButton,&QToolButton::pressed,this,&MainWindow::onCompareButtonClicked);
    connect(toolB.compareCurrentToLoadedFile,&QToolButton::clicked,this,&MainWindow::onCompareButtonClicked);
    connect(toolB.searchForDuplicates,&QToolButton::clicked,this,&MainWindow::on_scanDuplicates_pushButton_clicked);
    connect(toolB.countFileContent,&QToolButton::clicked,this,&MainWindow::onCountLinesButtonClicked);


}

MainWindow::~MainWindow()
{
    delete scanThread;
    delete ui;
}

/*void MainWindow::scanFolder(const std::string &folderPath)
{
    if(scanThread->isRunning())
        return;

    if(scanThread->setFolderPath(folderPath))
    {
        threadTaskUpdateTimer->start(500);
        scanThread->runScan();
    }
}*/

void MainWindow::onLoadPath()
{
    QString path = QFileDialog::getExistingDirectory(0,tr("Open directory"),".");
    if(path == "")
        return;
    ui->path_lineEdit->setText(path);
}
void MainWindow::on_scan_pushButton_clicked()
{
    qDebug()<<"on_scan_pushButton_clicked";
    if(scanThread->isRunning())
        return;

    goToPage(PageIndex::fileScanPage);

    if(scanThread->setFolderPath(ui->path_lineEdit->text().toStdString()))
    {
        qDebug() << "startScan2: "<<ui->path_lineEdit->text();
        //ui->md5Scan_progressBar->hide();
        //threadTaskUpdateTimer->start(500);
        //scanModel->clear();
        Folder *f = scanThread->getFolderResult();
        //setProgress(0);
        if(f)
        if(m_scanNeedsUpdate)
        {
            qDebug() << "path not equal: ";
            scanThread->reset();
            //scanModel->clear();
        }
        //scanThread->runScan();
        m_fileScanTreeView->scanFiles();
        //threadTaskUpdateTimer->stop();
        m_scanNeedsUpdate = false;
    }
}
void MainWindow::on_rescan_pushButton_clicked()
{
    if(scanThread->isRunning())
        return;

    goToPage(PageIndex::fileScanPage);
    scanThread->reset();
    on_scan_pushButton_clicked();
}
void MainWindow::on_scanMd5_pushButton_clicked()
{
    if(scanThread->isRunning())
        return;
    Folder *f = scanThread->getFolderResult();

    if(!f || m_scanNeedsUpdate)
    {
        qDebug() << "read the dir first";
        on_scan_pushButton_clicked();
        qDebug() << "Now analyze the files";
    }
    goToPage(PageIndex::fileScanPage);
    //threadTaskUpdateTimer->start(500);
    setProgress(0);
    m_fileScanTreeView->scanMD5();
    m_md5NeedsUpdate = false;
    //threadTaskUpdateTimer->stop();
}

void MainWindow::on_cancel_pushButton_clicked()
{
    scanThread->cancelScan();
}
void MainWindow::onThreadTaskUpdateTimer()
{
    return;
    switch(scanThread->getCurrentTask())
    {
        case Task::fileScan:
        {
          //  m_fileScanTreeView->updateTree();
            break;
        }
        case Task::md5Scan:
        case Task::lineCounter:
        {
            static double timePrediction = 1;
            double progress = scanThread->getMasterProgress();
            double neededProgress = 1-progress;

            double runtime = scanThread->getRuntime();
            timePrediction = runtime*neededProgress/progress;


            qDebug() << "Scan: "<<progress*100<<"%\t"<<timePrediction<<"s";

            if(progress != 0)
            {
                //ui->scanTimePrediction_label->setText(getTimeString(timePrediction).c_str());
                setProgress(progress,getTimeString(timePrediction));
            }
            else
                setProgress(0.01);
            break;
        }
        default:
        {}
    }


}

void MainWindow::onScanThreadResultFinish(ScanThread *thread, const Folder &folder)
{
    qDebug() << "scan finished. Time: "<<thread->getRuntime();
    //threadTaskUpdateTimer->stop();
    setProgress(1);
    //ui->scanTimePrediction_label->setText("");
    m_fileScanTreeView->updateTree();
}

void MainWindow::onExportButtonClicked()
{
    QString path = QFileDialog::getSaveFileName(0,tr("Save File"),".");
    qDebug() << "export: "<<path;
    Folder *f = scanThread->getFolderResult();
    if(f)
    if(f->exportToFile(path.toStdString()))
    {
        qDebug() << "export done";
        return;
    }
    qDebug() << "can't export file";
}
void MainWindow::onCompareButtonClicked()
{
    QString path = QFileDialog::getOpenFileName(0,tr("Load File"),".");
    std::ifstream s(path.toStdString(),std::ifstream::in);
    if(!s.is_open())
    {
        qDebug() << "can't open File: "<<path;
        return;
    }
    qDebug() << "load: "<<path;
    vector<string> contentA;
    contentA.reserve(5000);
    string line;
    while (std::getline(s, line))
    {
        if(line.find("F{") == string::npos &&
           line.find("Root{") == string::npos)
            continue;
        contentA.push_back(line);
    }
    s.close();
    qDebug() << "reading done";

    qDebug() << "export to table";
    //vector<string> contentB;
    //scanThread->getFolderResult()->exportToTable(contentB);



    qDebug() << "import from string";
    vector<FileData> fA = FileData::buildFromImport(contentA);
    vector<FileData> fB;
    Folder *f = scanThread->getFolderResult();
    if(!f || m_md5NeedsUpdate)
    {
        on_scanMd5_pushButton_clicked();
    }
    // Update the filePointer, it may be got destroyed because of possible rescan
    f = scanThread->getFolderResult();
    if(f)
        f->getAllFileDataRecursive(fB);
    if(fA.size() != fB.size())
       qDebug() << "Content size not equal";
    qDebug() << "compare";
    goToPage(PageIndex::compareScanPage);
    m_compareScanPage->compare(fA,fB);

}

void MainWindow::on_scanDuplicates_pushButton_clicked()
{
    if(scanThread->isRunning())
    {
        qDebug() << "scanning...";
        return;
    }
    Folder *f = scanThread->getFolderResult();
    if(!f || m_md5NeedsUpdate)
    {
        on_scanMd5_pushButton_clicked();
    }
    goToPage(PageIndex::duplicateScanPage);
    m_duplicateScanTreeView->scan();
}
void MainWindow::onCountLinesButtonClicked()
{
    if(scanThread->isRunning())
    {
        qDebug() << "scanning...";
        return;
    }
    Folder *f = scanThread->getFolderResult();
    if(!f || m_scanNeedsUpdate)
    {
        qDebug() << "read the dir first";
        on_scan_pushButton_clicked();
        qDebug() << "Now analyze the files";
    }
    goToPage(PageIndex::lineCountPage);
    //threadTaskUpdateTimer->start(500);
    //setProgress(0);
    m_lineCountPage->scan();
}
void MainWindow::onVersionButtonClicked()
{
    QMessageBox *box = new QMessageBox(this);
    box->setWindowTitle("Version");
    box->setInformativeText(m_versionsText.c_str());
    box->setStandardButtons(QMessageBox::Ok);
    box->show();
}

void MainWindow::setProgress(double percent,const string &message)
{
    ui->md5Scan_progressBar->setValue(percent*1000);
    ui->loadingText_label->setText(message.c_str());
}
void MainWindow::goToPage(PageIndex index)
{
    ui->tabWidget->setCurrentIndex(index);
}

void MainWindow::on_path_lineEdit_textChanged(const QString &arg1)
{
    m_scanNeedsUpdate = true;
    m_md5NeedsUpdate  = true;
}

