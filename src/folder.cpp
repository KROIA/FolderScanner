#include "folder.h"

#ifdef FOLDER_TRHEADED_SCAN
std::mutex Folder::m_mutex;
int Folder::m_threadActiveCount = 0;
bool Folder::m_threadExit = false;
vector<std::thread*> Folder::m_threadList;
size_t Folder::m_maxThreadCount = std::thread::hardware_concurrency();
bool Folder::m_threadsExecuted = true;

#endif

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
        m_master_progressCounter = nullptr;
    }
    else
    {
        m_masterFolder       = this;
        m_masterMutex        = new QMutex();
        m_master_folderCount = new size_t(0);
        m_master_fileCount   = new size_t(0);
        m_master_contentSize = new uintmax_t(0);
        m_master_escapeScan  = new bool(false);
        m_master_progressCounter = new uintmax_t(0);

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
    if(!m_parent)
    {
        delete m_master_folderCount;
        delete m_master_fileCount;
        delete m_master_contentSize;
        delete m_masterMutex;
    }
}

void Folder::clear()
{
    for(size_t i=0; i<m_childFolderList.size(); i++)
        delete m_childFolderList[i];

    m_childFolderList.clear();

    for(size_t i=0; i<m_containedFiles.size(); i++)
        delete m_containedFiles[i];

    m_containedFiles.clear();
    m_folderIsComplete = false;


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
double Folder::getMasterProgress() const
{
    if(m_masterFolder != this)
        return 0;
    uintmax_t nData;
    uintmax_t doneData;
    m_masterMutex->lock();
    nData    = *m_masterFolder->m_master_contentSize;
    doneData = *m_masterFolder->m_master_progressCounter;
    m_masterMutex->unlock();

    if(nData == 0)
        return 1;

    return (double)doneData/(double)nData;
}

void Folder::scanThreaded()
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
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring widestr = converter.from_bytes(m_path);
    const wchar_t* widecstr = widestr.c_str();
    m_folderIsComplete = true;

    //vector<string> directoriesToScan;
    m_childFolderList.reserve(1000);
    //vector<std::filesystem::directory_entry> dirEntries;
    vector<std::filesystem::directory_entry> fileEntries;

    fileEntries.reserve(1000);
    try
    {
        for (const auto & file : directory_iterator( std::filesystem::path(widecstr)))
        {
            if(file.is_directory())
            {

                // If folder is not already in buffer
                //string folderName = file.path().filename().string();
                //directoriesToScan.push_back(file.path().filename().string());
                Folder *childFolder;

                childFolder = new Folder(this);
                masterAddFolderCount(1);
                m_standardItems[0]->appendRow(childFolder->getStandardItem());
                childFolder->setName(file.path().filename().string());
                m_childFolderList.push_back(childFolder);




                /*if(!childFolder->m_folderIsComplete)
                {
                    childFolder->scan();

                    m_folderIsComplete &= childFolder->m_folderIsComplete;

                    if(childFolder->m_folderIsComplete)
                        incrementFolderSize(childFolder->size());
                }*/



            }
            else
            {
                fileEntries.push_back(file);
                /*string fileName = file.path().filename().string();

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

                uintmax_t size = containedFile->getSize();
                incrementFolderSize(size);
                masterAddContentSize(size);

                m_containedFiles.push_back(containedFile);*/

            }
            //sort();
           /* std::chrono::high_resolution_clock::time_point m_endTime = std::chrono::high_resolution_clock::now();
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
            }*/


        }
    }
    catch (...) {
        qDebug() << "ERROR In folder: "<<m_path.c_str() << " widestr: "<<widestr;
    }

    // Preparing threads
    vector<ThreadData*> threadData;
    size_t folderCount = m_childFolderList.size();
    size_t jobSize = folderCount/(10*m_maxThreadCount)+1;

    size_t threadData_beginPoint = 0;
    m_threadExit = false;
    m_threadActiveCount = 0;
    m_threadList.resize(m_maxThreadCount,nullptr);
    threadData.resize(m_maxThreadCount,nullptr);

    for(size_t i=0; i<m_maxThreadCount; ++i)
    {
        threadData[i] = new ThreadData{
                .threadIndex=i,
                .list=m_childFolderList.data(),
                .listSize=folderCount,
                .beginPoint=&threadData_beginPoint,
                .jobSize=jobSize,
                .exit=&m_threadExit,
                .threadsActiveCount=&m_threadActiveCount};
    }
    for(size_t i=0; i<m_maxThreadCount; ++i)
    {
        m_threadList[i] = new std::thread(scanThreadFunc,threadData[i]);
    }
    bool allFinished = false;
    int activeThreads = m_maxThreadCount;
    using namespace std::chrono;

    high_resolution_clock::time_point t3 = high_resolution_clock::now();

    do{
        //high_resolution_clock::time_point t2 = high_resolution_clock::now();
        high_resolution_clock::time_point t4 = high_resolution_clock::now();
        //duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        duration<double> time_span2 = duration_cast<duration<double>>(t4 - t3);

        if(processQEvents())
        {
#ifdef DEBUG_FAST_HASH_THREAD
            qDebug() << "still waiting for threads to finish. Running: "<<activeThreads;
#endif
        }
        if(time_span2.count() > 0.01)
        {
            t3 = t4;
            {
                std::unique_lock<std::mutex> lck (m_mutex);
                activeThreads = m_threadActiveCount;
            }
            if(activeThreads == 0)
                allFinished = true;
        }
    }while(!allFinished);

    for(size_t i=0; i<m_maxThreadCount; ++i)
    {
        m_threadList[i]->join();
        delete m_threadList[i];
        delete threadData[i];
        m_threadList.clear();
        threadData.clear();
    }
    //endSearch:
    for(size_t i=0; i<m_childFolderList.size(); ++i)
    {
        m_folderIsComplete &= m_childFolderList[i]->m_folderIsComplete;

        if(m_childFolderList[i]->m_folderIsComplete)
            incrementFolderSize(m_childFolderList[i]->size());
    }
    sort();
    if(m_folderIsComplete)
    {
        m_standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(m_contentSize)));
        m_standardItems[2]->setText(QString::number(m_contentSize));
    }


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
    scanInternal();
}
void Folder::scanInternal()
{
    clear();
    getPath();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring widestr = converter.from_bytes(m_path);
    const wchar_t* widecstr = widestr.c_str();
    m_folderIsComplete = true;
    //qDebug() << "Folder: "<<m_path.c_str()<<" shuld be complete";


    std::chrono::high_resolution_clock::time_point m_startTime = std::chrono::high_resolution_clock::now();
    try {
        for (const auto & file : directory_iterator( std::filesystem::path(widecstr)))
        {
            if(file.is_directory())
            {

                // If folder is not already in buffer
                string folderName = file.path().filename().string();
                Folder *childFolder;

                childFolder = new Folder(this);
                masterAddFolderCount(1);

                childFolder->setName(file.path().filename().string());



                //Folder *childFolder = new Folder(this);
                //m_standardItems[0]->appendRow(childFolder->getStandardItem());
                //childFolder->setName(file.path().filename().string());

                //if(!childFolder->m_folderIsComplete)
                {
                    childFolder->scanInternal();

                    m_folderIsComplete &= childFolder->m_folderIsComplete;

                    if(childFolder->m_folderIsComplete)
                        incrementFolderSize(childFolder->size());
                   // else
                       // qDebug() << "Folder: "<<file.path().filename().string().c_str()<<" not completed";
                }
                m_childFolderList.push_back(childFolder);
                m_standardItems[0]->appendRow(childFolder->getStandardItem());



            }
            else
            {
                string fileName = file.path().filename().string();

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

                uintmax_t size = containedFile->getSize();
                incrementFolderSize(size);
                masterAddContentSize(size);

                m_containedFiles.push_back(containedFile);

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
        qDebug() << "ERROR In folder: "<<m_path.c_str() << " widestr: "<<widestr;
    }

    endSearch:
    sort();
    if(m_folderIsComplete)
    {
        m_standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(m_contentSize)));
        m_standardItems[2]->setText(QString::number(m_contentSize));
       // qDebug() << "Folder: "<<m_path.c_str()<<"     complete";
    }else
    {
       // qDebug() << "Folder: "<<m_path.c_str()<<" not complete";
    }
}

void Folder::scanThreadFunc(ThreadData *data)
{
    if(!data)
        return;
#ifdef DEBUG_FAST_HASH_THREAD
    qDebug() << "starte Thread: "<<data->threadIndex;
    size_t fileProcessedCount = 0;
#endif

    static std::mutex mtx;
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        ++*data->threadsActiveCount;
    }
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    bool exit = false;
    while(1)
    {

        Folder **folderBegin = nullptr;
        size_t jobSize = data->jobSize;
        size_t currentBeginIndex = 0;

        {
            std::unique_lock<std::mutex> lck (mtx);
            if(*data->beginPoint >= data->listSize)
                goto exitPoint;
            currentBeginIndex = *data->beginPoint;
            folderBegin = data->list;

            *data->beginPoint += jobSize;

        }
        {
            std::unique_lock<std::mutex> lck (m_mutex);
            exit = *data->exit;
        }
        if(exit)
            goto exitPoint;
        folderBegin += currentBeginIndex;
        if(data->listSize < currentBeginIndex+jobSize)
            jobSize = data->listSize - currentBeginIndex;
#ifdef DEBUG_FAST_HASH_THREAD
        qDebug() << "T "<<data->threadIndex << " Hashe index "<<currentBeginIndex << " - " <<currentBeginIndex+jobSize <<" of " << data->listSize;
#endif
        //uintmax_t progress = 0;
        for(size_t i=0; i<jobSize; ++i)
        {
            //File::md5(*folderBegin[i]);
            folderBegin[i]->scanInternal();
#ifdef DEBUG_FAST_HASH_THREAD
            ++fileProcessedCount;
#endif
            //progress += folderBegin[i]->getSize();
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            if(time_span.count() > 1)
            {
                t1 = t2;
                {
                    std::unique_lock<std::mutex> lck (m_mutex);
                    exit = *data->exit;
                }
                if(exit)
                    goto exitPoint;
            }
        }
        //data->masterFolder->masterAddProgress(progress);
    }

    exitPoint:


#ifdef DEBUG_FAST_HASH_THREAD
    if(exit)
        qDebug() << "Thread: "<<data->threadIndex<<" abgebrochen. "<< " Files hashed: "<<fileProcessedCount;
    else
        qDebug() << "Thread: "<<data->threadIndex<<" wird beendet. Files hashed: "<<fileProcessedCount;
#endif
    {
        std::unique_lock<std::mutex> lck (m_mutex);
        --*data->threadsActiveCount;
    }
}

void Folder::scanMd5()
{
    if(this == m_masterFolder)
    {
        m_masterMutex->lock();
        *m_masterFolder->m_master_escapeScan = false;
        *m_masterFolder->m_master_progressCounter = 0;
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
    uintmax_t progress = 0;
    for(size_t i=0; i<m_containedFiles.size(); i++)
    {
        File::md5(*m_containedFiles[i]);
        progress += m_containedFiles[i]->getSize();
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
            m_startTime = m_endTime;
        }
    }
    masterAddProgress(progress);

    m_startTime = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<m_childFolderList.size(); i++)
    {
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
            m_startTime = m_endTime;
        }
        m_childFolderList[i]->scanMd5();
    }

    getMd5();


}
void Folder::resetProgress()
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_progressCounter = 0;
    m_masterMutex->unlock();
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
const vector<Folder*> &Folder::getSubFolder()
{
    return m_childFolderList;
}
const vector<File*> &Folder::getFiles()
{
    return m_containedFiles;
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

/*void Folder::readImportData(const vector<string> &data)
{
    if(data.find("D{") == string::npos)
        return;


}
vector<string> Folder::getExportData()
{
    //string data = "D{<"+;
    vector<string> data;

    for(size_t i=0; i<m_childFolderList.size(); ++i)
    {
        vector<string> tmpData = m_childFolderList[i]->getExportData();
        string tmpStr;
        for(size_t j=0; j<tmpData.size(); ++j)
        {
            tmpStr += tmpData[i] + "\n";
        }
        data.push_back(tmpStr);
    }
    return "F{<"+m_path+"><"+m_fileName+"><"+std::to_string(m_size)+"><"+m_md5+">}";
}*/
bool Folder::importFromFile(const string &file)
{
    std::ifstream s(file,std::ifstream::in);
    if(!s.is_open())
        return false;

    internalImportFromFile(s);

    s.close();
    return true;
}

bool Folder::exportToFile(const string &file)const
{
    std::ofstream  s(file, std::ofstream::out);
    if (!s)
    {
        std::cout << "failed to open " << file << '\n';
        return false;
    }
    size_t bak = FileData::getExportPathBegin();
    FileData::setExportPathBegin(m_masterFolder->m_path.size()+1);
    //FileData::setExportPathBegin(0);
    s<<"Root{<"+m_masterFolder->m_path+">\n";
    internalExportToFile(s);
    FileData::setExportPathBegin(bak);
    s.close();
    qDebug() << "export done";
    return true;
}
void Folder::exportToTable(vector<string> &table)const
{
    table.clear();
    if(m_master_folderCount)
        table.reserve(*m_master_folderCount);
    size_t bak = FileData::getExportPathBegin();
    FileData::setExportPathBegin(m_masterFolder->m_path.size()+1);
    internalExportToTable(table);
    FileData::setExportPathBegin(bak);
}
void Folder::getAllFileDataRecursive(vector<FileData> &list) const
{
    list.clear();
    if(m_master_folderCount)
        list.reserve(*m_masterFolder->m_master_fileCount);
    internalGetAllFilesRecursive(list);
}
void Folder::getAllFilesRecursive(vector<File*> &list) const
{
    list.clear();
    if(m_master_folderCount)
        list.reserve(*m_master_fileCount);
    internalGetAllFilesRecursive(list);
}

void Folder::getAllFoldersRecursive(vector<Folder*> &list)
{
    list.clear();
    if(m_master_folderCount)
        list.reserve(*m_master_folderCount);
    internalGetAllFoldersRecursive(list);
    list.push_back(this);
}
void Folder::internalGetAllFilesRecursive(vector<FileData> &list) const
{
    for(File* f:m_containedFiles)
        list.push_back(f->getData());
    for(Folder* f:m_childFolderList)
        f->internalGetAllFilesRecursive(list);
}
void Folder::internalGetAllFilesRecursive(vector<File*> &list) const
{
    for(File* f:m_containedFiles)
        list.push_back(f);
    for(Folder* f:m_childFolderList)
        f->internalGetAllFilesRecursive(list);
}
void Folder::internalGetAllFoldersRecursive(vector<Folder*> &list) const
{
    for(Folder* f:m_childFolderList)
    {
        list.push_back(f);
        f->internalGetAllFoldersRecursive(list);
    }
}
void Folder::internalImportFromFile(std::ifstream &s)
{

}
void Folder::internalExportToFile(std::ofstream  &s)const
{
    //static int depth = 0;
    //++depth;
    //qDebug( )<< "depth: "<<depth;
    for(size_t i=0; i<m_childFolderList.size(); ++i)
    {
        if(!m_childFolderList[i])
        {
            qDebug() << "Error: m_childFolderList[i] == nullptr";
            continue;
        }
        m_childFolderList[i]->internalExportToFile(s);
    }
    for(size_t i=0; i<m_containedFiles.size(); ++i)
    {
        if(!m_containedFiles[i])
        {
            qDebug() << "Error: m_containedFiles[i] == nullptr";
            continue;
        }
        string data = m_containedFiles[i]->getExportData();
        s << data.c_str() << "\n";
       // qDebug() << "EXP: "<< data.c_str();
    }
    //--depth;
}
void Folder::internalExportToTable(vector<string> &table)const
{
    for(size_t i=0; i<m_childFolderList.size(); ++i)
    {
        m_childFolderList[i]->internalExportToTable(table);
    }
    for(size_t i=0; i<m_containedFiles.size(); ++i)
    {
        table.push_back(m_containedFiles[i]->getExportData());
    }
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
void Folder::masterResetProgress()
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_progressCounter = 0;
    m_masterMutex->unlock();
}
void Folder::masterAddProgress(size_t increment)
{
    m_masterMutex->lock();
    *m_masterFolder->m_master_progressCounter += increment;
    m_masterMutex->unlock();
}
