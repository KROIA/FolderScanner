#include "compareScanPage.h"
#include "ui_comparescanpage.h"

CompareScanPage::CompareScanPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CompareScanPage)
{
    ui->setupUi(this);
    m_tableModel = new QStandardItemModel(this);

    m_updateTimer = new QTimer;
    connect(m_updateTimer,&QTimer::timeout,this,&CompareScanPage::onUpdateTimer);
    m_progressPercent = 0;
    m_progressStep = ProgressStep::none;
}

CompareScanPage::~CompareScanPage()
{
    //delete m_tableView;
    delete m_tableModel;
    delete ui;
}

void CompareScanPage::compare(vector<FileData> &A,
                              vector<FileData> &B)
{
    //bool result = true;
    m_progressPercent = 0;
    m_progressStep = ProgressStep::comparing;
    m_updateTimer->start(500);
    size_t matchCount = 0;
    struct Pair
    {
        FileData A;
        FileData B;
    };
    vector<Pair> missmatchList;
    vector<FileData> newFiles;
    vector<FileData> deletedFiles;
    vector<bool> newFileCheckList(B.size(),false);

    for(size_t a=0; a<A.size(); ++a)
    {
        bool fileAFound = false;
        for(size_t b=0; b<B.size(); ++b)
        {
            if(A[a].getName() == B[b].getName())
            {
                if(A[a].getPath() == B[b].getPath())
                {
                    ++matchCount;
                    fileAFound = true;
                    newFileCheckList[b] = true;
                    bool equal = true;
                    if(A[a].getSize() != B[b].getSize())
                    {
                        equal = false;
                    }
                    if(A[a].getMd5() != B[b].getMd5())
                    {
                        equal = false;
                    }
                    if(!equal)
                    {
                        missmatchList.push_back({A[a],B[b]});
                    }
                }
            }

        }
        if(!fileAFound)
            deletedFiles.push_back(A[a]);

        m_progressPercent = (double)a/(double)A.size();
        processQEvents();
    }

    newFiles.reserve(newFileCheckList.size());
    for(size_t i=0; i<newFileCheckList.size(); ++i)
        if(!newFileCheckList[i])
            newFiles.push_back(B[i]);

    m_progressPercent = 0;
    m_progressStep = ProgressStep::buildTableView;

    qDebug() << "A size: "<<A.size();
    qDebug() << "B size: "<<B.size();
    qDebug() << "MatchCount: "<<matchCount;
    qDebug() << "MissMatchCount: "<<missmatchList.size();
    vector<vector<string>> rawTable;

    rawTable.push_back({"index","Path A","Size A","Size bytes A","MD5 A",
                                "Path B","Size B","Size bytes B","MD5 B"});
    ui->tableView->setModel(nullptr);
    m_tableModel->clear();
    m_tableModel->setColumnCount(8);
    size_t tableRowCount =  missmatchList.size()+
                            deletedFiles.size()+
                            newFiles.size();
    m_tableModel->setRowCount(tableRowCount);

    ui->changedFilesLabel->setText(QString::number(missmatchList.size()));
    ui->deletedFilesLabel->setText(QString::number(deletedFiles.size()));
    ui->newFilesLabel->setText(QString::number(newFiles.size()));


    int column = 0;
    qDebug()<< "Set Header";
    m_tableModel->setHorizontalHeaderLabels({"Path A","Size A","Size bytes A","MD5 A",
                                             "Path B","Size B","Size bytes B","MD5 B"});

    qDebug()<< "Set Header done";




    size_t progressCounter = 0;
    for(size_t i=0; i<missmatchList.size(); ++i)
    {
        FileData *A = &missmatchList[i].A;
        FileData *B = &missmatchList[i].B;
        string fullPathA;
        string fullPathB;
        if(A->getPath().size() > 0)
            fullPathA = A->getPath()+"/";
        if(B->getPath().size() > 0)
            fullPathB = B->getPath()+"/";

        fullPathA+=A->getName();
        fullPathB+=B->getName();

        rawTable.push_back({std::to_string(i),fullPathA,FileData::sizeToStr(A->getSize()),std::to_string(A->getSize()),A->getMd5(),
                                                  fullPathB,FileData::sizeToStr(B->getSize()),std::to_string(B->getSize()),B->getMd5()});

        int column = 0;
        QModelIndex modelIndex;
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,fullPathA.c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,FileData::sizeToStr(A->getSize()).c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getSize());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getMd5().c_str());

        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,fullPathB.c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,FileData::sizeToStr(B->getSize()).c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,B->getSize());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,B->getMd5().c_str());

        m_progressPercent = (double)++progressCounter/(double)tableRowCount;
        processQEvents();
    }

    for(size_t i=0; i<deletedFiles.size(); ++i)
    {
        string fullPathA;
        if(deletedFiles[i].getPath().size() > 0)
            fullPathA = deletedFiles[i].getPath()+"/";
        fullPathA+=deletedFiles[i].getName();


        rawTable.push_back({std::to_string(i),fullPathA,FileData::sizeToStr(deletedFiles[i].getSize()),std::to_string(deletedFiles[i].getSize()),deletedFiles[i].getMd5(),
                                              "","",""});

        int column = 0;
        QModelIndex modelIndex;
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,fullPathA.c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,FileData::sizeToStr(deletedFiles[i].getSize()).c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,deletedFiles[i].getSize());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,deletedFiles[i].getMd5().c_str());


        m_progressPercent = (double)++progressCounter/(double)tableRowCount;
        processQEvents();
    }
    for(size_t i=0; i<newFiles.size(); ++i)
    {
        string fullPathB;
        if(newFiles[i].getPath().size() > 0)
            fullPathB = newFiles[i].getPath()+"/";
        fullPathB+=newFiles[i].getName();
        //string fullPathB = newFiles[i]->getPath()+"\\"+newFiles[i]->getName();
        rawTable.push_back({std::to_string(i),"","","",
                            fullPathB,FileData::sizeToStr(newFiles[i].getSize()),std::to_string(newFiles[i].getSize()),newFiles[i].getMd5()});
        int column = 3;
        QModelIndex modelIndex;
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,fullPathB.c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,FileData::sizeToStr(newFiles[i].getSize()).c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,newFiles[i].getSize());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,newFiles[i].getMd5().c_str());

        m_progressPercent = (double)++progressCounter/(double)tableRowCount;
        processQEvents();
    }


    vector<size_t> columnWidthList;
    vector<string> table = getEqualSpaced(rawTable,columnWidthList);

    ui->tableView->setModel(m_tableModel);

    for(size_t i=1; i<columnWidthList.size(); ++i)
    {
        ui->tableView->setColumnWidth(i-1,columnWidthList[i]*6);
    }
    for(int i=0; i<m_tableModel->rowCount(); ++i)
    {
        ui->tableView->setRowHeight(i,10);
    }

   // for(size_t i=0; i<table.size(); ++i)
   // {
   //     qDebug() <<table[i].c_str();
   // }
    //ui->stackedWidget->setCurrentIndex(1);


    m_progressPercent = 1;
    m_updateTimer->stop();
    emit setProgress(m_progressPercent,"");
    m_progressStep = ProgressStep::none;
}
void CompareScanPage::clear()
{
    m_tableModel->clear();
}

void CompareScanPage::onUpdateTimer()
{
    updateProgress();
}
void CompareScanPage::updateProgress()
{
    string text;
    switch(m_progressStep)
    {
        case ProgressStep::comparing:
        {
            text = "Comparing files...";
            break;
        }
        case ProgressStep::buildTableView:
        {
            text = "Building table...";
            break;
        }
        default:{}
    }
    emit setProgress(m_progressPercent,text);
}
