#include "folder.h"

Folder::Folder(Folder *parent)
{
    m_parent = parent;
    m_contentSize = 0;
    //m_folderNameItem = nullptr;
    m_folderIsComplete = false;
    if(m_parent != nullptr)
    {
        m_masterFolder = m_parent->m_masterFolder;
        m_masterMutex  = m_parent->m_masterMutex;
        m_master_folderCount = nullptr;
        m_master_fileCount   = nullptr;
        m_master_contentSize = nullptr;
        m_master_escapeScan  = nullptr;
        m_master_md5ProgressCounter = nullptr;
    }
    else
    {
        m_masterFolder       = this;
        m_masterMutex        = new QMutex();
        m_master_folderCount = new size_t(0);
        m_master_fileCount   = new size_t(0);
        m_master_contentSize = new uintmax_t(0);
        m_master_escapeScan  = new bool(false);
        m_master_md5ProgressCounter = new uintmax_t(0);

    }

    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems[0]->setEditable(false);
    m_standardItems[1]->setEditable(false);
    m_standardItems[2]->setEditable(false);
    m_standardItems[3]->setEditable(false);
}
Folder::~Folder()
{
    clear();
}

void Folder::clear()
{
    for(size_t i=0; i<m_childFolderList.size(); i++)
        delete m_childFolderList[i];

    for(size_t i=0; i<m_containedFiles.size(); i++)
        delete m_containedFiles[i];

    if(!m_parent)
    {
        delete m_master_folderCount;
        delete m_master_fileCount;
        delete m_master_contentSize;
        delete m_masterMutex;
    }
   // if(m_folderNameItem)
   //    delete m_folderNameItem;
}

void Folder::setName(const string &name)
{
    m_name = name;
    m_standardItems[0]->setText(QString::fromStdString(m_name));
}
const string &Folder::getName() const
{
    return m_name;
}
string Folder::getPath()
{
    if(m_parent == nullptr)
    {
        m_path = m_name;
        return m_path;
    }
    m_path = m_parent->getPath()+"\\"+m_name;
    return m_path;
}

size_t Folder::getMasterFolderCount() const
{
    return *m_masterFolder->m_master_folderCount;
}
size_t Folder::getMasterFileCount() const
{
    return *m_masterFolder->m_master_fileCount;
}
uintmax_t Folder::getMasterContentSize() const
{
    return *m_masterFolder->m_master_contentSize;
}
double Folder::getMasterMD5ScanProgress() const
{
    if(m_masterFolder != this)
        return 0;
    uintmax_t nData;
    uintmax_t doneData;
    m_masterMutex->lock();
    nData    = *m_masterFolder->m_master_contentSize;
    doneData = *m_masterFolder->m_master_md5ProgressCounter;
    m_masterMutex->unlock();

    if(nData == 0)
        return 1;

    return (double)doneData/(double)nData;
}

void Folder::scan()
{
    if(m_folderIsComplete)
        return;
    if(this == m_masterFolder)
    {
        m_masterMutex->lock();
        *m_masterFolder->m_master_escapeScan = false;
        m_masterMutex->unlock();
        masterAddFolderCount(1);
    }
    else
    {
        // Breaks recursive loop if flag is set
        m_masterMutex->lock();
        bool esc = *m_masterFolder->m_master_escapeScan;
        m_masterMutex->unlock();
        if(esc)
            return;
    }


    getPath();
    //qDebug() << "scanning Folder: "<<m_path.c_str();
    std::wstring widestr = std::wstring(m_path.begin(), m_path.end());
    const wchar_t* widecstr = widestr.c_str();
    m_folderIsComplete = true;


    std::unordered_map<std::string, Folder*> folderMap;
    std::unordered_map<std::string, File*> fileMap;
    for(size_t i=0; i<m_childFolderList.size(); i++)
    {
        //if(!m_childFolderList[i]->m_folderIsComplete)
        {
            //m_childFolderList[i]->scan();
            //m_folderIsComplete &= m_childFolderList[i]->m_folderIsComplete;
            folderMap.insert({m_childFolderList[i]->getName(),m_childFolderList[i]});
        }
    }
    for(size_t i=0; i<m_containedFiles.size(); i++)
    {
        fileMap.insert({m_containedFiles[i]->getName(),m_containedFiles[i]});
    }



    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    try {
        for (const auto & file : directory_iterator( std::filesystem::path(widecstr)))
        {
            if(file.is_directory())
            {

                // If folder is not already in buffer
                string folderName = file.path().filename().string();
                Folder *childFolder;
                if(folderMap[folderName] == nullptr)
                {
                    childFolder = new Folder(this);
                    masterAddFolderCount(1);
                    m_standardItems[0]->appendRow(childFolder->getStandardItem());
                    childFolder->setName(file.path().filename().string());
                    m_childFolderList.push_back(childFolder);
                }
                else
                {
                    childFolder = folderMap[folderName];
                }

                //Folder *childFolder = new Folder(this);
                //m_standardItems[0]->appendRow(childFolder->getStandardItem());
                //childFolder->setName(file.path().filename().string());

                if(!childFolder->m_folderIsComplete)
                {
                    childFolder->scan();

                    m_folderIsComplete &= childFolder->m_folderIsComplete;

                    if(childFolder->m_folderIsComplete)
                        incrementFolderSize(childFolder->size());
                }



            }
            else
            {
                string fileName = file.path().filename().string();
                if(fileMap[fileName] == nullptr)
                {
                   // qDebug() << "scanning File: "<<file.path().c_str();
                    masterAddFileCount(1);
                    File *containedFile = new File(file);
                    m_masterFolder->m_masterMutex->lock();
                    int rowCount = m_standardItems[0]->rowCount();
                    m_masterFolder->m_masterMutex->unlock();
                    for(int i=0; i<rowCount; i++)
                    {
                        m_masterFolder->m_masterMutex->lock();
                        string str = m_standardItems[0]->child(i)->data().toString().toStdString();
                        m_masterFolder->m_masterMutex->unlock();
                    }
                    m_masterFolder->m_masterMutex->lock();
                    m_standardItems[0]->appendRow(containedFile->getStandardItem());
                    m_masterFolder->m_masterMutex->unlock();

                    uintmax_t size = containedFile->size();
                    incrementFolderSize(size);
                    masterAddContentSize(size);

                    m_containedFiles.push_back(containedFile);
                }
            }
            //sort();
            std::chrono::high_resolution_clock::time_point m_endTime = std::chrono::high_resolution_clock::now();
            auto m_time_span = std::chrono::duration_cast<std::chrono::duration<double> >(m_endTime - m_startTime);
            // Check if the loop must be canceled
            if(m_time_span.count() > 1) // 1 sec.
            {
                // Breaks recursive loop if flag is set
                m_masterMutex->lock();
                bool esc = *m_masterFolder->m_master_escapeScan;
                m_masterMutex->unlock();
                if(esc)
                {
                    goto endSearch;
                }
                 m_startTime = std::chrono::high_resolution_clock::now();
            }


        }
    }  catch (...) {
        qDebug() << "ERROR In folder: "<<m_path.c_str();
    }

/*
 * std::sort(list.begin(),list.end(),[ ]( const Folder* lhs, const Folder* rhs )
    {
       return lhs->size() > rhs->size();
    });
 * */
    endSearch:
    sort();
    if(m_folderIsComplete)
    {
        m_standardItems[1]->setText(QString::fromStdString(File::sizeToStr(m_contentSize)));
        m_standardItems[2]->setText(QString::number(m_contentSize));
    }




    //m_standardItems[0]->sortChildren(2);

    /*m_masterMutex->lock();
    m_folderIsComplete = true;
    m_masterMutex->unlock();*/


    // std::cout<<"Folders: "<<folderCount() << " Files: "<<fileCount() << "\n";
}
void Folder::scanMd5()
{
    if(this == m_masterFolder)
    {
        m_masterMutex->lock();
        *m_masterFolder->m_master_escapeScan = false;
        *m_masterFolder->m_master_md5ProgressCounter = 0;
        m_masterMutex->unlock();
    }
    else
    {
        // Breaks recursive loop if flag is set
        m_masterMutex->lock();
        bool esc = *m_masterFolder->m_master_escapeScan;
        m_masterMutex->unlock();
        if(esc)
            return;
    }

    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<m_containedFiles.size(); i++)
    {
        m_containedFiles[i]->getMd5();
        masterAddMd5Progress(m_containedFiles[i]->size());
        std::chrono::high_resolution_clock::time_point m_endTime = std::chrono::high_resolution_clock::now();
        auto m_time_span = std::chrono::duration_cast<std::chrono::duration<double> >(m_endTime - m_startTime);
        // Check if the loop must be canceled
        if(m_time_span.count() > 1) // 1 sec.
        {
            // Breaks recursive loop if flag is set
            m_masterMutex->lock();
            bool esc = *m_masterFolder->m_master_escapeScan;
            m_masterMutex->unlock();
            if(esc)
                return;
            m_startTime = std::chrono::high_resolution_clock::now();
        }
    }

    for(size_t i=0; i<m_childFolderList.size(); i++)
        m_childFolderList[i]->scanMd5();

    getMd5();


}
void Folder::cancelScan()
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_escapeScan = true;
    m_masterMutex->unlock();
}

uintmax_t Folder::size() const
{
    return m_contentSize;
}
size_t Folder::fileCount() const
{
    return m_containedFiles.size();
}
size_t Folder::folderCount() const
{
    return m_childFolderList.size();
}
const string Folder::getMd5()
{
    if(m_md5 == "")
        return md5(*this);
    return m_md5;
}



void Folder::print(size_t tab)
{
    string tabs;
    for(size_t i=0; i<tab; ++i)
        tabs+="  ";
    std::cout <<tabs<<getName()<<" "<<size()<<" bytes\n";

    ++tab;
    for(size_t i=0; i<m_childFolderList.size(); i++)
        m_childFolderList[i]->print(tab);

    for(size_t i=0; i<m_containedFiles.size(); i++)
        m_containedFiles[i]->print(tab);
}

void Folder::sortBySize(Folder *owner,vector<Folder*> &list)
{
    /*vector<QList<QStandardItem *>   > itemList;
    int rowCount = owner->m_standardItems[0]->rowCount();
    for(int i=0; i<rowCount; i++)
    {
        itemList.push_back(owner->m_standardItems[0]->takeRow(0));
    }


    std::sort(itemList.begin(),itemList.end(),[ ]( const QList<QStandardItem *>  lhs, const QList<QStandardItem *>   rhs )
    {
       return lhs[2]->data().toULongLong() > rhs[2]->data().toULongLong();
    });

    for(size_t i=0; i<itemList.size(); i++)
    {
        owner->m_standardItems[0]->appendRow(itemList[i]);
    }*/


    std::sort(list.begin(),list.end(),[ ]( const Folder* lhs, const Folder* rhs )
    {
       return lhs->size() > rhs->size();
    });


}
bool Folder::operator > (const Folder &folder) const
{
    return size() > folder.size();
}
bool Folder::operator < (const Folder &folder) const
{
    return size() < folder.size();
}

QList<QStandardItem *> Folder::getStandardItem() const
{
    /*bool finished;
    m_masterMutex->lock();
    finished = m_folderIsComplete;
    m_masterMutex->unlock();
    if(!finished)
        return QList<QStandardItem *>();*/

    //if(m_folderNameItem == nullptr)
    //    m_folderNameItem = new QStandardItem(QString::fromStdString(getName()));
    /*QStandardItem *m_folderNameItem = new QStandardItem(QString::fromStdString(getName()));

    for(size_t i=0; i<m_childFolderList.size(); i++)
        m_folderNameItem->appendRow(m_childFolderList[i]->getStandardItem());

    for(size_t i=0; i<m_containedFiles.size(); i++)
        m_folderNameItem->appendRow(m_containedFiles[i]->getStandardItem());



    return {m_folderNameItem,
            new QStandardItem(QString::fromStdString(File::sizeToStr(size()))),
            new QStandardItem(QString::number(size())),
            new QStandardItem(QString::fromStdString(m_md5))};*/
    return m_standardItems;
}
bool Folder::dirExists(const std::string& dirName_in)
{
  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  //something is wrong with your path!

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;   // this is a directory!

  return false;    // this is not a directory!
}
string Folder::md5 (Folder& folder) {

    QByteArray filesHashes = "";
    for(size_t i=0; i<folder.m_containedFiles.size(); i++)
    {
        filesHashes.append(folder.m_containedFiles[i]->getMd5());
    }
    for(size_t i=0; i<folder.m_childFolderList.size(); i++)
    {
        filesHashes.append(folder.m_childFolderList[i]->getMd5());
    }

    QByteArray hashData = QCryptographicHash::hash(filesHashes,QCryptographicHash::Md5); // or QCryptographicHash::Sha1
    folder.m_md5 = hashData.toHex().toStdString();
    folder.m_standardItems[3]->setText(QString::fromStdString(folder.m_md5));
    return folder.m_md5;
}

void Folder::incrementFolderSize(uintmax_t size)
{
    m_contentSize += size;
    /*m_masterFolder->m_masterMutex->lock();
    try {
        m_standardItems[1]->setData(QString::fromStdString(File::sizeToStr(m_contentSize)),Qt::DisplayRole);
        m_standardItems[2]->setData(m_contentSize,Qt::DisplayRole);
    }  catch (...) {
        qDebug() << "catchError";
    }*/

    //m_masterFolder->m_masterMutex->unlock();
    //m_standardItems[1]->setText(QString::fromStdString(File::sizeToStr(m_contentSize)));
    //m_standardItems[2]->setText(QString::number(m_contentSize));
    //if(m_parent)
    //    m_parent->incrementFolderSize(size);
}
void Folder::sort()
{
    sortBySize(this, m_childFolderList);
    File::sortBySize(m_containedFiles);

    int rowCount = m_standardItems[0]->rowCount();
    for(int i=0; i<rowCount; i++)
    {
        m_standardItems[0]->takeRow(0);
    }
    for(size_t i=0; i<m_childFolderList.size(); i++)
    {
        m_standardItems[0]->appendRow(m_childFolderList[i]->getStandardItem());
    }
    for(size_t i=0; i<m_containedFiles.size(); i++)
    {
        m_standardItems[0]->appendRow(m_containedFiles[i]->getStandardItem());
    }
}


void Folder::masterAddFolderCount(size_t increment)
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_folderCount += increment;
    m_masterMutex->unlock();
}
void Folder::masterAddFileCount(size_t increment)
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_fileCount += increment;
    m_masterMutex->unlock();
}
void Folder::masterAddContentSize(uintmax_t increment)
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_contentSize += increment;
    m_masterMutex->unlock();
}
void Folder::masterAddMd5Progress(size_t increment)
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_md5ProgressCounter += increment;
    m_masterMutex->unlock();
}
