#include "folder.h"

Folder::Folder(Folder *parent)
{
    m_parent = parent;
    m_contentSize = 0;
   // m_folderIsComplete = false;
    if(m_parent != nullptr)
    {
        m_masterFolder = m_parent->m_masterFolder;
        m_masterMutex  = m_parent->m_masterMutex;
        m_master_folderCount = nullptr;
        m_master_fileCount   = nullptr;
        m_master_contentSize = nullptr;
        m_master_escapeScan  = nullptr;
    }
    else
    {
        m_masterFolder       = this;
        m_masterMutex        = new QMutex();
        m_master_folderCount = new size_t(0);
        m_master_fileCount   = new size_t(0);
        m_master_contentSize = new uintmax_t(0);
        m_master_escapeScan  = new bool(false);

    }
}
Folder::~Folder()
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
}

void Folder::setName(const string &name)
{
    m_name = name;
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

void Folder::scan()
{
    if(this == m_masterFolder)
    {
        m_masterMutex->lock();
        *m_masterFolder->m_master_escapeScan = false;
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


    getPath();
    //qDebug() << "scanning Folder: "<<m_path.c_str();
    std::wstring widestr = std::wstring(m_path.begin(), m_path.end());
    const wchar_t* widecstr = widestr.c_str();

    try {
        for (const auto & file : directory_iterator( std::filesystem::path(widecstr)))
        {
            if(file.is_directory())
            {
                masterAddFolderCount(1);
                Folder *childFolder;
                childFolder = new Folder(this);
                childFolder->setName(file.path().filename().string());
                childFolder->scan();
                m_contentSize += childFolder->size();

                m_childFolderList.push_back(childFolder);                
            }
            else
            {
               // qDebug() << "scanning File: "<<file.path().c_str();
                masterAddFileCount(1);
                File *containedFile = new File(file);
                uintmax_t size = containedFile->size();
                m_contentSize += size;
                masterAddContentSize(size);

                m_containedFiles.push_back(containedFile);
            }
        }
    }  catch (...) {
        qDebug() << "ERROR In folder: "<<m_path.c_str();
    }


    sortBySize(m_childFolderList);
    File::sortBySize(m_containedFiles);

    /*m_masterMutex->lock();
    m_folderIsComplete = true;
    m_masterMutex->unlock();*/


    // std::cout<<"Folders: "<<folderCount() << " Files: "<<fileCount() << "\n";
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

void Folder::sortBySize(vector<Folder*> &list)
{
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

    QStandardItem *folderNameItem = new QStandardItem(QString::fromStdString(getName()));

    for(size_t i=0; i<m_childFolderList.size(); i++)
        folderNameItem->appendRow(m_childFolderList[i]->getStandardItem());

    for(size_t i=0; i<m_containedFiles.size(); i++)
        folderNameItem->appendRow(m_containedFiles[i]->getStandardItem());



    return {folderNameItem,
            new QStandardItem(QString::fromStdString(File::sizeToStr(size()))),
            new QStandardItem(QString::number(size()))};
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
