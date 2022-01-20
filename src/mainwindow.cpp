#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , standardModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    frame = new QScrollArea(ui->displayFrame);
    //frame->setGeometry(0,60,800,900);
    QRect geometry = ui->displayFrame->geometry();
    geometry.setX(0);
    geometry.setY(0);
    geometry.setWidth(geometry.width()-50);
    frame->setGeometry(geometry);

    //frame->resize(200,500);
    frame->setBackgroundRole(QPalette::Dark);
    //frame->adjustSize();
    //setCentralWidget(frame);

    treeView = new QTreeView();
    treeView->setGeometry(QRect(0,0,frame->width(),treeView->height()));
    frame->setWidget(treeView);

    scanUpdateTimer = new QTimer(this);
    connect(scanUpdateTimer,&QTimer::timeout,this,&MainWindow::onScanUpdateTimer);
    scanMd5UpdateTimer = new QTimer(this);
    connect(scanMd5UpdateTimer,&QTimer::timeout,this,&MainWindow::onScanMd5UpdateTimer);

    scanThread = new ScanThread();
    connect(scanThread, &ScanThread::resultReady, this, &MainWindow::onScanThreadResultFinish);
    //connect(scanThread, &ScanThread::finished, scanThread, &MainWindow::deleteLater);

    ui->md5Scan_progressBar->hide();
}

MainWindow::~MainWindow()
{
    //scanThread->quit();
    //scanThread->wait();
    delete scanThread;
    delete ui;
}

void MainWindow::scanFolder(const std::string &folderPath)
{
    /*if(!Folder::dirExists(folderPath))
    {
        qDebug() << "path not found";
        QMessageBox::critical(this, tr("File scanner"),
                                    tr("This path is not valid\n"),
                                    QMessageBox::Ok);
        return;
    }*/

    if(scanThread->isRunning())
        return;


    if(scanThread->setFolderPath(folderPath))
    {
        ui->md5Scan_progressBar->hide();

        scanUpdateTimer->start(50);
        scanThread->runScan();

    }




    /*Folder folder;
    folder.setName(folderPath);
    folder.scan();*/

  //  folder.print();



    //QList<QStandardItem *> preparedRow = prepareRow("first", "second");


    //treeView->expandAll();

}
void MainWindow::updateUIstats()
{
    size_t folderCount;
    size_t fileCount;
    uintmax_t contentSize;
    double runtime;

    scanThread->lockThread();
    folderCount = scanThread->getMasterFolderCount();
    fileCount   = scanThread->getMasterFileCount();
    contentSize = scanThread->getMasterContentSize();
    runtime = scanThread->getRuntime();
    scanThread->unlockThread();


    qDebug() << "update: Folders: "<<folderCount<<"\tFiles: "<<fileCount<<"\tSize: "<<File::sizeToStr(contentSize).c_str()<<"\tTime: "<<runtime;
    ui->foldersLabel->setText(QString::number(folderCount));
    ui->filesLabel->setText(QString::number(fileCount));
    ui->timeLabel->setText(QString::number(runtime,'g',2)+"s");
    ui->sizeLabel->setText(File::sizeToStr(contentSize).c_str());
}
void MainWindow::updateUITreeView()
{
    QStandardItem *item = standardModel->invisibleRootItem();
    if(item->rowCount() == 0)
    {
        item->appendRow(scanThread->getFolderResult().getStandardItem());
        standardModel->setHeaderData(0,Qt::Horizontal, "Name");
        standardModel->setHeaderData(1,Qt::Horizontal, "Grösse");
        standardModel->setHeaderData(2,Qt::Horizontal, "Grösse [bytes]");
        standardModel->setHeaderData(3,Qt::Horizontal, "MD5 Hash");

        treeView->setModel(standardModel);
        treeView->setColumnWidth(0,300);

        if(item->hasChildren())
        {
            treeView->expand(item->child(0,0)->index());
        }
        //standardModel->horizontalHeaderItem(0)->setSelectable(true);
        //standardModel->header().setSortIndicatorShown(True)
        //standardModel->header().sortIndicatorChanged.connect(self.model.sort)
    }
    //standardModel->sort(2);



}
void MainWindow::on_scan_pushButton_clicked()
{
    //scanFolder(ui->path_lineEdit->text().toStdString());
    if(scanThread->isRunning())
        return;


    if(scanThread->setFolderPath(ui->path_lineEdit->text().toStdString()))
    {
        ui->md5Scan_progressBar->hide();
        scanUpdateTimer->start(50);
        scanThread->runScan();

    }
}
void MainWindow::on_rescan_pushButton_clicked()
{
    if(scanThread->isRunning())
        return;
    scanThread->reset();
    standardModel->clear();
    on_scan_pushButton_clicked();
}

void MainWindow::on_scanMd5_pushButton_clicked()
{
    if(scanThread->isRunning())
        return;
    scanMd5UpdateTimer->start(50);
    ui->md5Scan_progressBar->setValue(0);
    ui->md5Scan_progressBar->show();
    scanThread->runMD5Scan();

}

void MainWindow::on_cancel_pushButton_clicked()
{
    scanThread->cancelScan();
   // scanUpdateTimer->stop();
   // scanMd5UpdateTimer->stop();
}


void MainWindow::onScanUpdateTimer()
{
    updateUIstats();
    updateUITreeView();
}
void MainWindow::onScanMd5UpdateTimer()
{
    static double timePrediction = 1;
    double progress = scanThread->getMasterMD5ScanProgress();
    double neededProgress = 1-progress;

    double runtime = scanThread->getRuntime();
    //timePrediction = 0.9*timePrediction + 0.1* (runtime*100/progress);
    timePrediction = runtime*neededProgress/progress;


    qDebug() << "Scan MD5: "<<progress<<"%\t"<<timePrediction<<"s";
    ui->md5Scan_progressBar->setValue(progress*1000);
    ui->md5ScanTimePrediction_label->setText(QString::number(timePrediction)+"s");
}
void MainWindow::onScanThreadResultFinish(ScanThread *thread, const Folder &folder)
{
    updateUIstats();
    qDebug() << "scan finished. Time: "<<thread->getRuntime();
    scanUpdateTimer->stop();
    scanMd5UpdateTimer->stop();
    ui->md5Scan_progressBar->setValue(1000);
    updateUITreeView();

}







