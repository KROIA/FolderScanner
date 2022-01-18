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

    scanThread = new ScanThread();
    connect(scanThread, &ScanThread::resultReady, this, &MainWindow::onScanThreadResultFinish);
    //connect(scanThread, &ScanThread::finished, scanThread, &MainWindow::deleteLater);
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
        scanUpdateTimer->start(50);
        scanThread->run();
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
    standardModel->clear();
    QStandardItem *item = standardModel->invisibleRootItem();
    item->appendRow(scanThread->getFolderResult().getStandardItem());

    // adding a row to the invisible root item produces a root element
    //item->appendRow(preparedRow);

    //QList<QStandardItem *> secondRow = prepareRow("111", "222");
    // adding a row to an item starts a subtree
    //preparedRow[0]->appendRow(secondRow);

    standardModel->setHeaderData(0,Qt::Horizontal, "Name");
    standardModel->setHeaderData(1,Qt::Horizontal, "Grösse");
    standardModel->setHeaderData(2,Qt::Horizontal, "Grösse [bytes]");

    treeView->setModel(standardModel);
    treeView->setColumnWidth(0,300);

    if(item->hasChildren())
    {
        treeView->expand(item->child(0,0)->index());
    }
}
void MainWindow::on_scan_pushButton_clicked()
{
    scanFolder(ui->path_lineEdit->text().toStdString());
}
void MainWindow::on_cancel_pushButton_clicked()
{
    scanThread->cancelScan();
}

void MainWindow::onScanUpdateTimer()
{
    updateUIstats();
    updateUITreeView();
}
void MainWindow::onScanThreadResultFinish(ScanThread *thread, const Folder &folder)
{
    updateUIstats();
    qDebug() << "scan finished. Time: "<<thread->getRuntime();
    scanUpdateTimer->stop();
    updateUITreeView();

}





