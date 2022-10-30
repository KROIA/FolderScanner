#include "lineCountPage.h"
#include "ui_lineCountPage.h"

const string LineCountPage::m_noExtentionStr = "noExtention";
LineCountPage::LineCountPage(ScanThread* scanThread,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LineCountPage)
{
    ui->setupUi(this);
    m_tableModel = new QStandardItemModel(this);
    m_treeView = new TreeViewPage(ui->displayFrame);
    m_scanThread = scanThread;
    connect(m_scanThread,&ScanThread::onReset,this,&LineCountPage::clear);

    QVBoxLayout  *layout = new QVBoxLayout ;
    ui->fileExtentionFilter_scrollArea->widget()->setLayout(layout);
    m_scanFilter.force_fileExtentionWhiteList = true;

    m_updateTimer = new QTimer;
    connect(m_updateTimer,&QTimer::timeout,this,&LineCountPage::onUpdateTimer);

    m_progressPercent = 0;
    m_progressStep = ProgressStep::none;
    ui->filterWidget->setEnabled(false); // disable the filter area for the processing task
}

LineCountPage::~LineCountPage()
{
    //delete m_tableView;
    delete m_treeView;
    delete m_tableModel;
    delete ui;
}

void LineCountPage::setScanFilter(const ScanFilter &filter)
{
    m_scanFilter = filter;
}
const ScanFilter &LineCountPage::getScanFilter()
{
    return m_scanFilter;
}

void LineCountPage::scan()
{
    Folder *f = m_scanThread->getFolderResult();
    if(!f || m_scanThread->isRunning())
        return;

    //m_progressText = "Reading Files...";
    //m_progressText = "";
    m_updateTimer->start(100);
    m_progressStep = ProgressStep::readingFiles;
    ui->filterWidget->setEnabled(false); // disable the filter area for the processing task
    //updateProgress();


    qDebug() << "LineCountPage::updatePage() m_scanThread->runLineCounter();";
    m_scanThread->runLineCounter();
    updateScannedFiles();


    m_updateTimer->stop();
}
void LineCountPage::sortFilterCheckBoxes(vector<QCheckBox*> &list)
{
    // Sort alphabetic
    std::sort(list.begin(),list.end(), [ ]( const QCheckBox* lhs, const QCheckBox* rhs )
    {
       return lhs->text().toStdString() < rhs->text().toStdString();
    });
}
void LineCountPage::updateScannedFiles()
{
    Folder *f = m_scanThread->getFolderResult();

    m_progressStep = ProgressStep::filtering;
    ui->filterWidget->setEnabled(false); // disable the filter area for the processing task
    m_progressPercent = 0;
    updateFileExtentionCheckBoxFilter();
    updateFilter();

    m_progressStep = ProgressStep::buildTableView;
   // m_progressText = "Building Table...";
    qDebug() << "LineCountPage::updatePage() Building Table...";
    //f = m_scanThread->getFolderResult();
    buildTable(f);

    m_progressStep = ProgressStep::buildTreeView;
    qDebug() << "LineCountPage::updatePage() Building Tree...";
    buildTree(f);


    ui->filterWidget->setEnabled(true);
    emit setProgress(1,"");
    qDebug() << "LineCountPage::updatePage() finish";
    m_progressStep = ProgressStep::none;
}
void LineCountPage::clear()
{
    m_tableModel->clear();
    m_treeView->clear();
}
void LineCountPage::onFilterCheckBoxClicked(bool state)
{
    //QObject *sender = QObject::sender();
    //updateFilter();
    Folder*f=m_scanThread->getFolderResult();
    if(f)
    if(f->size() < 1'000'000'000)// > 1GB
        on_applyFilter_pushButton_clicked();
}
void LineCountPage::on_applyFilter_pushButton_clicked()
{
    m_updateTimer->start(100);
    updateScannedFiles();
    m_updateTimer->stop();
}
void LineCountPage::on_filterEnableNone_pushButton_clicked()
{
    for(auto b:m_extentionFilterCheckBoxList)
        b.second->setChecked(false);
    Folder*f=m_scanThread->getFolderResult();
    if(f)
    if(f->size() < 1'000'000'000)// > 1GB
        on_applyFilter_pushButton_clicked();
}
void LineCountPage::on_filterEnableAll_pushButton_clicked()
{
    for(auto b:m_extentionFilterCheckBoxList)
        b.second->setChecked(true);
    Folder*f=m_scanThread->getFolderResult();
    if(f)
    if(f->size() < 1'000'000'000)// > 1GB
        on_applyFilter_pushButton_clicked();
}
void LineCountPage::on_filterSearch_lineEdit_textChanged(const QString &arg1)
{
    string search = arg1.toLower().toStdString();
    if(search.size() == 0)
    {
        vector<QCheckBox*> allButtonsList;
        allButtonsList.reserve(m_extentionFilterCheckBoxList.size());
        for(auto &pair:m_extentionFilterCheckBoxList)
        {
            ui->fileExtentionFilter_scrollArea->widget()->layout()->removeWidget(pair.second);
            allButtonsList.push_back(pair.second);
        }
        sortFilterCheckBoxes(allButtonsList);
        for(auto &box:allButtonsList)
        {
            ui->fileExtentionFilter_scrollArea->widget()->layout()->addWidget(box);
            box->show();
        }
    }
    else
    {
        vector<QCheckBox*> filteredButtonList;
        filteredButtonList.reserve(m_extentionFilterCheckBoxList.size());

        for(auto &pair:m_extentionFilterCheckBoxList)
        {
            ui->fileExtentionFilter_scrollArea->widget()->layout()->removeWidget(pair.second);
            pair.second->hide();
            if(pair.second->text().toLower().indexOf(search.c_str()) > -1)
                filteredButtonList.push_back(pair.second);
            //if(pair.second->text().indexOf(search.c_str()) == -1)
            //    ui->fileExtentionFilter_scrollArea->widget()->layout()->removeWidget(pair.second);
        }
        sortFilterCheckBoxes(filteredButtonList);
        for(auto &box:filteredButtonList)
        {

            ui->fileExtentionFilter_scrollArea->widget()->layout()->addWidget(box);
            box->show();
        }
       // ui->fileExtentionFilter_scrollArea->widget()->layout()->update();
    }
}

void LineCountPage::onUpdateTimer()
{
    updateProgress();
    /*if(m_progressText.size() == 0)
    {
        static double timePrediction = 1;
        double progress = m_scanThread->getMasterProgress();
        double neededProgress = 1-progress;

        double runtime = m_scanThread->getRuntime();
        timePrediction = runtime*neededProgress/progress;


        qDebug() << "Scan: "<<progress*100<<"%\t"<<timePrediction<<"s";
            m_progressText = getTimeString(timePrediction);

        if(progress > 0.01)
        {
            //ui->scanTimePrediction_label->setText(getTimeString(timePrediction).c_str());
            setProgress(progress,getTimeString(timePrediction));
        }
        else
            setProgress(0.01);
    }*/
}

void LineCountPage::updateProgress()
{
    string text;
    switch(m_progressStep)
    {
        case ProgressStep::readingFiles:
        {
            m_progressPercent = m_scanThread->getMasterProgress();
            text = "Reading files...";

            static double timePrediction = 1;
            //double progress = m_scanThread->getMasterProgress();
            double neededProgress = 1-m_progressPercent;

            double runtime = m_scanThread->getRuntime();
            timePrediction = runtime*neededProgress/m_progressPercent;


            //qDebug() << "Scan: "<<m_progressPercent*100<<"%\t"<<timePrediction<<"s";
            if(m_progressPercent > 0.01)
                text += "\n"+getTimeString(timePrediction);

            break;
        }
        case ProgressStep::filtering:
        {
            text = "Applying filter...";
            break;
        }
        case ProgressStep::buildTableView:
        {
            text = "Building table...";
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
void LineCountPage::buildTable(Folder *mainFolder)
{
    m_progressPercent = 0;
    vector<vector<string>> rawTable;
    vector<FileData> rawData;
    mainFolder->getAllFileDataRecursive(rawData);

    vector<FileData> data;
    getFiltered(rawData, data, m_scanFilter);

    //rawTable.push_back({"Path","Size","Size bytes","Lines","MD5"});

    ui->tableView->setModel(0);
    m_tableModel->clear();
    m_tableModel->setColumnCount(6);
    m_tableModel->setRowCount(data.size());
    rawTable.reserve(data.size());




    //qDebug()<< "Set Header";
    m_tableModel->setHorizontalHeaderLabels({"Path","Size","Size bytes","Lines","Characters","MD5"});

    //qDebug()<< "Set Header done";


    for(size_t i=0; i<data.size(); ++i)
    {
        FileData *A = &data[i];
        string fullPathA;
        if(A->getPath().size() > 0)
            fullPathA = A->getPath()+"/";

        fullPathA+=A->getName();

        rawTable.push_back({fullPathA,FileData::sizeToStr(A->getSize()),std::to_string(A->getSize()),
                            std::to_string(A->getLineCount()),std::to_string(A->getCharCount()), A->getMd5()});

        int column = 0;
        QModelIndex modelIndex;
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,fullPathA.c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,FileData::sizeToStr(A->getSize()).c_str());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getSize());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getLineCount());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getCharCount());
        modelIndex = m_tableModel->index(i,column++,QModelIndex());   m_tableModel->setData(modelIndex,A->getMd5().c_str());

        processQEvents();
        m_progressPercent = (double)i/(double)data.size();
    }
    vector<size_t> columnWidthList;
    vector<string> table = getEqualSpaced(rawTable,columnWidthList);
    ui->tableView->setModel(m_tableModel);
    for(size_t i=0; i<columnWidthList.size(); ++i)
    {
        ui->tableView->setColumnWidth(i,columnWidthList[i]*6);
    }
    for(int i=0; i<m_tableModel->rowCount(); ++i)
    {
        ui->tableView->setRowHeight(i,10);
    }

    m_progressPercent = 1;
    updateProgress();
}
void LineCountPage::buildTree(Folder *mainFolder)
{
    m_progressPercent = 0;
    QList<QStandardItem*> tree;
    //tree.reserve(4);
    tree.push_back(new QStandardItem());
    /*tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    tree.push_back(new QStandardItem());
    //tree.push_back(new QStandardItem());
    //tree.push_back(new QStandardItem());
    tree[0]->setEditable(false);
    tree[1]->setEditable(false);
    tree[2]->setEditable(false);
    tree[3]->setEditable(false);
    //tree[4]->setEditable(false);
    //tree[5]->setEditable(false);
    tree[0]->setText("File");
    tree[1]->setText("Size");
    tree[2]->setText("Lines");
    tree[3]->setText("Characters");*/
    uintmax_t byteCount = 0;
    uintmax_t lineCount = 0;
    uintmax_t charCount = 0;
    m_treeFolderCounter = 0;
    m_treeFolderCounterMax = mainFolder->folderCount();
    buildTreeRecursive(mainFolder,tree,byteCount,lineCount,charCount);
    m_treeView->clear();

    m_treeView->setTree(tree[0]->takeRow(0),
                        {"Name","Size","Lines","Characters"});
    m_progressPercent = 1;
    updateProgress();
}
void LineCountPage::buildTreeRecursive(Folder *folder, QList<QStandardItem*> &tree,
                                           uintmax_t &bytes, uintmax_t &lineCount, uintmax_t &charCount)
{
    string path = folder->getPath();
    if(path.size()>0)
        path+="/";
    path+=folder->getName();
    if(!isInFilter(path,"",
                  //m_scanFilter.dirWhiteList,
                  m_scanFilter.dirBlackList,
                    {},{},m_scanFilter.force_fileExtentionWhiteList))
        return; // this Folder is not in the Filter

    QList<QStandardItem*> thisFolderTree;
    thisFolderTree.reserve(4);
    thisFolderTree.push_back(new QStandardItem());
    thisFolderTree.push_back(new QStandardItem());
    thisFolderTree.push_back(new QStandardItem());
    thisFolderTree.push_back(new QStandardItem());
    //thisFolderTree.push_back(new QStandardItem());
    //thisFolderTree.push_back(new QStandardItem());
    thisFolderTree[0]->setEditable(false);
    thisFolderTree[1]->setEditable(false);
    thisFolderTree[2]->setEditable(false);
    thisFolderTree[3]->setEditable(false);
    //thisFolderTree[4]->setEditable(false);
    //thisFolderTree[5]->setEditable(false);
    thisFolderTree[0]->setText(QString::fromStdString(folder->getName()));


    //thisFolderTree[4]->setText(QString::fromStdString(p[0].getMd5()));


    for(Folder *subF:folder->getSubFolder())
    {
        uintmax_t bc = 0;
        uintmax_t lc = 0;
        uintmax_t cc = 0;
        buildTreeRecursive(subF,thisFolderTree,bc,lc,cc);
        bytes     += bc;
        lineCount += lc;
        charCount += cc;
    }
    for(File *f:folder->getFiles())
    {
        FileData fd = f->getData();
        if(!isInFilter(fd,m_scanFilter))
            continue;
        QList<QStandardItem*> standardItems;

        standardItems.reserve(4);
        standardItems.push_back(new QStandardItem());
        standardItems.push_back(new QStandardItem());
        standardItems.push_back(new QStandardItem());
        standardItems.push_back(new QStandardItem());
        //standardItems.push_back(new QStandardItem());
        //standardItems.push_back(new QStandardItem());
        standardItems[0]->setEditable(false);
        standardItems[1]->setEditable(false);
        standardItems[2]->setEditable(false);
        standardItems[3]->setEditable(false);
        //standardItems[4]->setEditable(false);
        standardItems[0]->setText(QString::fromStdString(fd.getName()));
        standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(fd.getSize())));
        standardItems[2]->setText(QString::number(fd.getLineCount()));
        standardItems[3]->setText(QString::number(fd.getCharCount()));

        thisFolderTree[0]->appendRow(standardItems);

        bytes     += fd.getSize();
        lineCount += fd.getLineCount();
        charCount += fd.getCharCount();

    }

    thisFolderTree[1]->setText(QString::fromStdString(FileData::sizeToStr(bytes)));
    thisFolderTree[2]->setText(QString::number(lineCount));
    thisFolderTree[3]->setText(QString::number(charCount));

    if(bytes != 0)
        tree[0]->appendRow(thisFolderTree);

    m_progressPercent = (double)++m_treeFolderCounter/(double)m_treeFolderCounterMax;
}

void LineCountPage::getFiltered(const vector<FileData> &rawIn,
                                vector<FileData> &out,
                                const ScanFilter &filter)
{
    out.clear();
    out.reserve(rawIn.size());
    for(const FileData &f:rawIn)
    {
        if(isInFilter(f,filter))
            out.push_back(f);
    }
}
bool LineCountPage::isInFilter(const FileData &file, const ScanFilter &filter)
{
    return isInFilter(file.getPath(),file.getName(),
                      //filter.dirWhiteList,
                      filter.dirBlackList,
                      filter.fileExtentionWhiteList,
                      filter.fileExtentionBlackList,
                      filter.force_fileExtentionWhiteList);
}
bool LineCountPage::isInFilter(const string &path,const string &fileName,
                               //const vector<string> &dirWhiteList,
                               const vector<string> &dirBlackList,
                               const vector<string> &fileExtentionWhiteList,
                               const vector<string> &fileExtentionBlackList,
                               bool force_fileExtentionWhiteList)
{
    //string path = file.getPath();
    for(size_t i=0; i<dirBlackList.size(); ++i)
    {

        string blackList = dirBlackList[i];
        string::size_type startPos = path.find(blackList);
        bool matchStart = false;
        bool matchEnd = false;
        if(startPos != string::npos)
        {
           //qDebug() << "Filter found in "<<path.c_str();
            if(startPos > 0)
            {
                if(path[startPos-1] != '/' &&
                   path[startPos-1] != '\\' &&
                   path[startPos-1] != ':')
                {
                    // Filter does not include the whole folder name
                    continue;
                }
                else{matchStart=true;}
            }else{matchStart=true;}
            if(startPos+blackList.size() < path.size())
            {
                //qDebug() << "blacklist A";
                if(path[startPos+blackList.size()] != '/' &&
                   path[startPos+blackList.size()] != '\\' &&
                   path[startPos+blackList.size()] != '\0')
                {
                    // Filter does not include the whole folder name
                    //qDebug() << "blacklist B"<<path[startPos+blackList.size()];
                    continue;
                }else{matchEnd=true;}
            }else{matchEnd=true;}
        }
        if(matchStart && matchEnd)
        {
            //qDebug() << "blacklist match";
            return false; // Blacklist match
        }
    }
/*
    if(dirWhiteList.size() > 0)
    {
        for(size_t i=0; i<dirWhiteList.size(); ++i)
        {

            string whiteList = dirWhiteList[i];
            string::size_type startPos = path.find(whiteList);
            bool matchStart = false;
            bool matchEnd = false;
            if(startPos != string::npos)
            {
                qDebug() << "Filter found in "<<path.c_str();
                if(startPos > 0)
                {
                    if(path[startPos-1] != '/' &&
                       path[startPos-1] != '\\' &&
                       path[startPos-1] != ':')
                    {
                        // Filter does not include the whole folder name
                        continue;
                    }
                    else{qDebug() <<"begin match";matchStart=true;}
                }else{matchStart=true;}
                if(startPos+whiteList.size() < path.size())
                {
                    if(path[startPos+whiteList.size()] != '/' &&
                       path[startPos+whiteList.size()] != '\\' &&
                       path[startPos+whiteList.size()] != '\0')
                    {
                        // Filter does not include the whole folder name
                        continue;
                    }else{qDebug() <<"end match";matchEnd=true;}
                }else{matchEnd=true;}
            }
            if(matchStart && matchEnd)
                goto fileExtentionFilter; // Whitelist match
        }
        qDebug() << "no whitelist match";
        return false;
    }

    fileExtentionFilter:*/

    for(size_t i=0; i<fileExtentionBlackList.size(); ++i)
    {
        string::size_type pos = fileName.rfind(fileExtentionBlackList[i]);
        if(pos == fileName.size()-fileExtentionBlackList[i].size())
        {
           // qDebug() << "Blacklist match";
            return false; // Blacklist match
        }
        if(fileExtentionBlackList[i] == m_noExtentionStr && fileName.rfind(".") == string::npos)
        {
            return false; // Blacklist match
        }
    }
    if((fileExtentionWhiteList.size() > 0 || force_fileExtentionWhiteList) &&
            fileName.size() > 0)                   // Name must be defined
    {
        for(size_t i=0; i<fileExtentionWhiteList.size(); ++i)
        {
            string::size_type pos = fileName.rfind(fileExtentionWhiteList[i]);
            if(pos == fileName.size()-fileExtentionWhiteList[i].size())
            {
             //   qDebug() << "Whitelist match";
                return true; // Whitelist match
            }
            if(fileExtentionWhiteList[i] == m_noExtentionStr && fileName.rfind(".") == string::npos)
            {
                return true; // Whitelist match
            }
        }
       // qDebug() << "No Whitelist match";
        return false;
    }
    //qDebug() << "match";
    return true;
}


void LineCountPage::updateFileExtentionCheckBoxFilter()
{
    vector<string> ext;
    getAllFileExtentions(ext);
    std::unordered_map<string,QCheckBox*> extentionFilterCheckBoxList_copy = m_extentionFilterCheckBoxList;
    vector<QCheckBox*> allreadyInList;
    vector<QCheckBox*> willStayInList;
    vector<QCheckBox*> allButtonsList;

    allreadyInList.reserve(extentionFilterCheckBoxList_copy.size());
    for(auto b:extentionFilterCheckBoxList_copy)
        allreadyInList.push_back(b.second);

    willStayInList.reserve(extentionFilterCheckBoxList_copy.size());
    allButtonsList.reserve(ext.size());

    m_extentionFilterCheckBoxList.clear();
    for(size_t i=0; i<ext.size(); ++i)
    {
        if(extentionFilterCheckBoxList_copy.find(ext[i]) != extentionFilterCheckBoxList_copy.end())
        {
            QCheckBox* box = extentionFilterCheckBoxList_copy[ext[i]];
            willStayInList.push_back(box);
            m_extentionFilterCheckBoxList[ext[i]] = box;
            allButtonsList.push_back(box);
            continue; // Extention already in List
        }
        QCheckBox *box = new QCheckBox;
        box->setText(ext[i].c_str());
        box->setChecked(true);
        connect(box,&QCheckBox::clicked,this,&LineCountPage::onFilterCheckBoxClicked);
        m_extentionFilterCheckBoxList[ext[i]] = box;
        allButtonsList.push_back(box);
    }

    vector<QCheckBox*> toDeleteList;
    toDeleteList.reserve(allreadyInList.size());
    for(size_t i=0; i<allreadyInList.size(); ++i)
    {
        bool match = false;
        for(size_t j=0; j<willStayInList.size(); ++j)
        {
            if(allreadyInList[i] == willStayInList[j])
                match = true;
        }
        if(!match)
            toDeleteList.push_back(allreadyInList[i]);
    }

    for(size_t i=0; i<allreadyInList.size(); ++i)
    {
        ui->fileExtentionFilter_scrollArea->widget()->layout()->removeWidget(allreadyInList[i]);
        allreadyInList[i]->hide();
    }

    for(size_t i=0; i<toDeleteList.size(); ++i)
    {
        delete toDeleteList[i];
    }


    sortFilterCheckBoxes(allButtonsList);

    for(size_t i=0; i<allButtonsList.size(); ++i)
    {
        ui->fileExtentionFilter_scrollArea->widget()->layout()->addWidget(allButtonsList[i]);
        allButtonsList[i]->show();
    }
    on_filterSearch_lineEdit_textChanged(ui->filterSearch_lineEdit->text());
    m_progressPercent = 0.5;



}
void LineCountPage::updateFilter()
{
    m_scanFilter.fileExtentionWhiteList.clear();
    m_scanFilter.fileExtentionWhiteList.reserve(m_extentionFilterCheckBoxList.size());

    size_t c = 0;
    for(auto pair:m_extentionFilterCheckBoxList)
    {
        QCheckBox *box = pair.second;
        if(box->isChecked())
        {
            // Remove the "*" from the text: "*.txt"
            string extendtion = box->text().mid(1).toStdString();
            m_scanFilter.fileExtentionWhiteList.push_back(extendtion);
        }
        ++c;
        m_progressPercent = 0.5+(double)c/((double)m_extentionFilterCheckBoxList.size()*2);
    }
}
void LineCountPage::getAllFileExtentions(vector<string> &ext)
{
    Folder *f = m_scanThread->getFolderResult();
    if(!f)
        return;
    ext.clear();
    vector<FileData> files;
    f->getAllFileDataRecursive(files);
    std::unordered_map<string,string> extMap;

    for(size_t i=0; i<files.size(); ++i)
    {
        string ext = files[i].getExtention();
        if(ext.size() == 0)
            ext = m_noExtentionStr;

        ext = "*."+ext;
        extMap[ext] = ext;
    }

    ext.reserve(extMap.size());
    for(auto pair:extMap)
    {
        ext.push_back(pair.second);
    }

}












