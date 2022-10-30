#include "duplicateScanPage.h"
#include "ui_duplicateScanPage.h"

DuplicateScanPage::DuplicateScanPage(ScanThread* scanThread, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DuplicateScanPage)
{
    ui->setupUi(this);
    m_treeView = new TreeViewPage(ui->duplicateDisplay);
    m_scanThread = scanThread;
    connect(m_scanThread,&ScanThread::onReset,this,&DuplicateScanPage::clear);

    m_updateTimer = new QTimer;
    connect(m_updateTimer,&QTimer::timeout,this,&DuplicateScanPage::onUpdateTimer);

    m_progressPercent = 0;
    m_progressStep = ProgressStep::none;
}

DuplicateScanPage::~DuplicateScanPage()
{
    delete m_treeView;
    delete ui;
}

void DuplicateScanPage::scan()
{
    Folder *f = m_scanThread->getFolderResult();
    if(!f || m_scanThread->isRunning())
        return;

    vector<FileData> content;
    m_updateTimer->start(100);
    m_progressPercent = 0.01;

    f->getAllFileDataRecursive(content);
    searchDuplicates(content);

    m_updateTimer->stop();
    emit setProgress(1,"");
    m_progressStep = ProgressStep::none;
}

void DuplicateScanPage::clear()
{
    m_treeView->clear();
}
void DuplicateScanPage::onUpdateTimer()
{
    updateProgress();
}

void DuplicateScanPage::updateProgress()
{
    string text;
    switch(m_progressStep)
    {
        case ProgressStep::matchDuplicates:
        {
            text = "Searching for duplicates...";
            break;
        }
        case ProgressStep::buildTreeView:
        {
            text = "Building treeview...";
            break;
        }
        default:{}
    }

    emit setProgress(m_progressPercent,text);
}

void DuplicateScanPage::searchDuplicates(vector<FileData> &data)
{

    m_progressPercent = 0;
    m_progressStep = ProgressStep::matchDuplicates;
    std::unordered_map<std::string, vector<FileData>> duplicates;
    vector<bool> alreadyMatched(data.size(),false);


    for(size_t a=0; a<data.size(); ++a)
    {
        for(size_t b=a+1; b<data.size(); ++b)
        {
            if(!alreadyMatched[b])
            if(data[a].getMd5() == data[b].getMd5())
            if(data[a].getSize() == data[b].getSize())
            {
                alreadyMatched[b] = true;
                if(duplicates.find(data[a].getMd5()) != duplicates.end())
                {
                    // Key exists
                    duplicates[data[a].getMd5()].push_back(data[b]);

                }
                else
                {
                    duplicates[data[a].getMd5()] = {data[a],data[b]};
                }
            }
        }
        m_progressPercent = (double)a/(double)data.size();
        //emit setProgress(progress);
        processQEvents();
    }
    vector<vector<FileData>> duplicatesList;
    duplicatesList.reserve(duplicates.size());
    for(auto &v:duplicates)
    {
        if(v.second.size() > 0)
            duplicatesList.push_back(v.second);
    }
//#define SORT_BY_OCURANCE

#ifdef SORT_BY_OCURANCE
    std::sort(duplicatesList.begin(),duplicatesList.end(), [ ]( const Pair &lhs, const Pair &rhs )
    {
       return lhs.duplicateCount > rhs.duplicateCount;
    });
#else
    std::sort(duplicatesList.begin(),duplicatesList.end(), [ ]( const vector<FileData> &lhs, const vector<FileData> &rhs )
    {
       return (lhs.size()*lhs[0].getSize()) > (rhs.size()*rhs[0].getSize());
    });
#endif

    qDebug() << "Duplicates: "<<duplicatesList.size();


    m_progressPercent = 0;
    m_progressStep = ProgressStep::buildTreeView;
    QList<QStandardItem*> tree;


    tree.reserve(6);
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree[0]->setEditable(false);
    tree[1]->setEditable(false);
    tree[2]->setEditable(false);
    tree[3]->setEditable(false);
    tree[4]->setEditable(false);
    tree[5]->setEditable(false);
    tree[0]->setText("Duplicates");
    //tree[1]->setText("");
    //tree[2]->setText("");
    //tree[3]->setText("");
    tree[4]->setText("");

    uintmax_t sumBytes = 0;
    uintmax_t sumWastedBytes = 0;
    for(size_t k=0; k<duplicatesList.size(); ++k)
    {
        vector<FileData> &p = duplicatesList[k];
        QList<QStandardItem*> fileTree;
        fileTree.reserve(6);
        fileTree.push_back(new QStandardItem());
        fileTree.push_back(new QStandardItem());
        fileTree.push_back(new QStandardItem());
        fileTree.push_back(new QStandardItem());
        fileTree.push_back(new QStandardItem());
        fileTree.push_back(new QStandardItem());
        fileTree[0]->setEditable(false);
        fileTree[1]->setEditable(false);
        fileTree[2]->setEditable(false);
        fileTree[3]->setEditable(false);
        fileTree[4]->setEditable(false);
        fileTree[5]->setEditable(false);
        fileTree[0]->setText(QString::fromStdString(p[0].getName()+" ["+std::to_string(p.size())+"]"));
        fileTree[1]->setText(QString::fromStdString(FileData::sizeToStr(p[0].getSize()*p.size())));
        fileTree[2]->setText(QString::number(p[0].getSize()));
        fileTree[3]->setText(QString::fromStdString(FileData::sizeToStr(p[0].getSize()*(p.size()-1) )));
        fileTree[4]->setText(QString::fromStdString(p[0].getMd5()));
        sumBytes += p[0].getSize()*p.size();
        sumWastedBytes += p[0].getSize()*(p.size()-1);

        //qDebug()<< p[0].getName().c_str();
        for(size_t i=0; i<p.size(); ++i)
        {
            string fullPath;
            if(p[i].getPath().size() > 0)
                fullPath = p[i].getPath()+"\\";
            fullPath+= p[i].getName();
            QList<QStandardItem*> standardItems;
            standardItems.reserve(6);
            standardItems.push_back(new QStandardItem());
            standardItems.push_back(new QStandardItem());
            standardItems.push_back(new QStandardItem());
            standardItems.push_back(new QStandardItem());
            standardItems.push_back(new QStandardItem());
            standardItems.push_back(new QStandardItem());
            standardItems[0]->setEditable(false);
            standardItems[1]->setEditable(false);
            standardItems[2]->setEditable(false);
            standardItems[3]->setEditable(false);
            standardItems[4]->setEditable(false);
            standardItems[0]->setText(QString::fromStdString(p[i].getName()));
            standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(p[i].getSize())));
            standardItems[2]->setText(QString::number(p[i].getSize()));
            standardItems[5]->setText(fullPath.c_str());

            //qDebug()<< p[i].getName().c_str();

            fileTree[0]->appendRow(standardItems);
        }
        tree[0]->appendRow(fileTree);
        m_progressPercent = (double)k/(double)duplicatesList.size();
        processQEvents();

    }
    tree[1]->setText(FileData::sizeToStr(sumBytes).c_str());
    tree[2]->setText(QString::number(sumBytes));
    tree[3]->setText(FileData::sizeToStr(sumWastedBytes).c_str());

     m_treeView->clear();
     m_treeView->setTree(tree,
                         {"Name","size","Size [bytes]","Wasted","MD5 Hash","Full Path"});
     //updateUIDuplicateListView(tree,{"Name","size","Size [bytes]","Wasted","MD5 Hash","Full Path"});
     m_progressPercent = 1;
}
