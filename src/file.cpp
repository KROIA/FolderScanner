#include "file.h"

size_t FileData::m_exportPathStartIndex = 0;
FileData::FileData()
{
    m_size = 0;
    m_linesCounted = false;
    m_lineCount = 0;
    m_charCount = 0;
}
#ifdef USE_BOOST_FILESYSTEM
FileData::FileData(const boost::filesystem::directory_entry &entry)
{
    //m_dirEntry = entry;
    m_fileName = entry.path().filename().string();
    m_path = entry.path().string();
    m_path = m_path.substr(0,m_path.rfind(m_fileName)-1);
    m_size = boost::filesystem::file_size(entry.path());
    setPath(m_path);
    m_linesCounted = false;
    m_lineCount = 0;
    m_charCount = 0;
}
#endif
FileData::FileData(const std::filesystem::directory_entry &entry)
{
    //m_dirEntry = entry;
    m_fileName = entry.path().filename().string();
    m_path = entry.path().string();
    m_path = m_path.substr(0,m_path.rfind(m_fileName)-1);
    setPath(m_path);
    m_size = entry.file_size();
    m_linesCounted = false;
    m_lineCount = 0;
    m_charCount = 0;
}
FileData::FileData(const FileData &other)
{
    m_size      = other.m_size;
    m_fileName  = other.m_fileName;
    m_path      = other.m_path;
    m_md5       = other.m_md5;

    m_linesCounted = other.m_linesCounted;
    m_lineCount = other.m_lineCount;
    m_charCount = other.m_charCount;
}
FileData::~FileData()
{

}

void FileData::setName(const string &name)
{
    m_linesCounted = false;
    m_fileName = name;
}
const string &FileData::getName() const
{
    return m_fileName;
}
string FileData::getExtention() const
{
    string::size_type pos = m_fileName.rfind(".");
    if(pos != string::npos)
    {
        return m_fileName.substr(pos+1);
    }
    return "";
}
void FileData::setPath(const string &path)
{
    m_linesCounted = false;
    m_path = path;
    if(m_path.find("\\") != string::npos)
        m_path = replaceAll(m_path,"\\","/");
}
const string &FileData::getPath() const
{
    return m_path;
}

void FileData::setSize(uintmax_t size)
{
    m_size = size;
}
uintmax_t FileData::getSize() const
{
    return m_size;
}
void FileData::setLineCount(uintmax_t lineCount)
{
    m_linesCounted = true;
    m_lineCount = lineCount;
}
uintmax_t FileData::getLineCount()
{
    if(!m_linesCounted)
        countLines();
    return m_lineCount;
}
void FileData::setCharCount(uintmax_t charCount)
{
    m_linesCounted = true;
    m_charCount = charCount;
}
uintmax_t FileData::getCharCount()
{
    if(!m_linesCounted)
        countLines();
    return m_charCount;
}

void FileData::setMd5(const string &md5)
{
    m_md5 = md5;
}
const string &FileData::getMd5()
{
    if(m_md5 == "")
        return md5(*this);
    return m_md5;
}

void FileData::print(size_t tab)
{
    string tabs;
    for(size_t i=0; i<tab; ++i)
        tabs+="  ";
    std::cout <<tabs<<getName()<<" "<<getSize()<<" bytes\n";
}

void FileData::sortBySize(vector<FileData*> &list)
{
    std::sort(list.begin(),list.end(), [ ]( const FileData* lhs, const FileData* rhs )
    {
       return lhs->getSize() > rhs->getSize();
    });
}
bool FileData::operator > (const FileData &FileData) const
{
    return getSize() > FileData.getSize();
}
bool FileData::operator < (const FileData &FileData) const
{
    return getSize() < FileData.getSize();
}
bool FileData::isEqual(FileData &other,bool useMd5, bool useName)
{
    if(useMd5)
    {
        if(m_md5 == "")
            md5(*this);

        if(other.m_md5 == "")
            md5(other);

        if(m_md5 != other.m_md5)
            return false;
    }
    if(getName() != other.getName() && useName)
        return false;

    return true;
}


string FileData::sizeToStr(uintmax_t size)
{
    string str;
    long double dSize = static_cast<long double>(size);

    unsigned int exponent = log10l(dSize);

    unsigned int kPotenz = exponent /3;
    if(kPotenz > 6)
        kPotenz = 6;

    dSize /= pow(1000,kPotenz);

    str = std::to_string(dSize);
    if(str.find(".") != std::string::npos)
        str = str.substr(0,str.find(".")+2);


    string unit;
    switch(kPotenz)
    {
        case 0:
            unit = "bytes";
        break;
        case 1:
            unit = "kB";
        break;
        case 2:
            unit = "MB";
        break;
        case 3:
            unit = "GB";
        break;
        case 4:
            unit = "TB";
        break;
        case 5:
            unit = "PB";
        break;
        case 6:
            unit = "EB";
        break;
    }
    str+=" "+unit;
    return str;
}

string const &FileData::md5(FileData& file) {

    string path;
    if(file.m_path.size()>0)
        path=file.m_path+"/";
    path+=file.m_fileName;
    QFile _file(QString::fromStdString(path));

    bool slicedHash = true;
    size_t buffLen = 100'000'000;
    if(file.m_size < buffLen)
        slicedHash = false;
        //buffLen = file.m_size;
    size_t buffBegin = 0;
    bool finish = false;
    //size_t iteration = 0;
    using namespace std::chrono;


    if(_file.open(QIODevice::ReadOnly))
    {
        if(slicedHash)
        {
            QByteArray fileData(buffLen,0);
            while (!finish)
            {
                //QByteArray fileData = _file.readAll();

                qint64 ret = _file.read(fileData.begin()+buffBegin,buffLen-buffBegin);
                if(ret > 0)
                {
                    //if(iteration)
                    //    qDebug() << "Iteration: "<<iteration;
                    //if(ret != buffLen)
                    //    memset(fileData.begin()+ret,0,(buffLen-ret)*sizeof(char));
                    QByteArray hashData = QCryptographicHash::hash(fileData,QCryptographicHash::Md5); // or QCryptographicHash::Sha1
                    file.m_md5 = hashData.toHex().toStdString();
                    buffBegin = file.m_md5.size();
                    memcpy(fileData.data(),file.m_md5.data(),buffBegin*sizeof(char));
                    //++iteration;
                }
                else
                    finish = true;

                processQEvents();

               // qDebug() << "MD5: "<<file.m_md5.c_str()<<"\t of: "<<file.getPath().c_str();
            }
        }
        else
        {
            QByteArray fileData = _file.readAll();
            QByteArray hashData = QCryptographicHash::hash(fileData,QCryptographicHash::Md5); // or QCryptographicHash::Sha1
            file.m_md5 = hashData.toHex().toStdString();
        }
    }
    else
    {
        qDebug() << "Can't open file to calculate the MD5 hash. File: "<<path.c_str();
    }
    return file.m_md5;
}
void FileData::countLines(FileData& file)
{
    file.countLines();
}

bool FileData::readImportData(string data)
{
    if(data.find("F{") == string::npos)
        return false;

    string param = extractValue(data,"p<",">");
    if(param.size() > 0)
        setPath(param);
    //int start = data.find("F{<")+3;
    //int end = data.find(">")-start;
    //setPath(data.substr(start,end)); data = data.substr(data.find(">")+1);

    param = extractValue(data,"n<",">");
    if(param.size() > 0)
        setName(param);

    //start = data.find("<")+1;
    //end = data.find(">")-start;
    //setName(data.substr(start,end)); data = data.substr(data.find(">")+1);

    param = extractValue(data,"s<",">");
    if(param.size() > 0)
        setSize(atoll(param.c_str()));

    //start = data.find("<")+1;
    //end = data.find(">")-start;
    //string sizeStr = data.substr(start,end); data = data.substr(data.find(">")+1);
    //setSize(atoll(sizeStr.c_str()));

    param = extractValue(data,"h<",">");
    if(param.size() > 0)
        setMd5(param);

    //start = data.find("<")+1;
    //end = data.find(">")-start;
    //setMd5(data.substr(start,end));
    param = extractValue(data,"l<",">");
    if(param.size() > 0)
        setLineCount(atoll(param.c_str()));
    param = extractValue(data,"c<",">");
    if(param.size() > 0)
        setCharCount(atoll(param.c_str()));
    return true;
}
FileData FileData::buildFromImport(const string &data)
{
    FileData f;
    f.readImportData(data);
    return f;
}
vector<FileData> FileData::buildFromImport(const vector<string> &dataList)
{
    string root;
    vector<FileData> files;

    files.reserve(dataList.size());
    for(const string &s:dataList)
    {
        if(s.find("Root{<")!=string::npos)
        {
            int begin = s.find("Root{<")+6;
            int end = s.find(">")-begin;
            root = s.substr(begin,end);
        }
        FileData f = buildFromImport(s);
        if(f.m_fileName.size()>0)
            files.push_back(f);
    }
    if(root.size() > 0)
    {
        for(FileData &d:files)
        {
            if(d.m_path.size() > 0)
                d.m_path = root+"/"+d.m_path;
            else
                d.m_path = root;
        }
    }
    return files;
}
string FileData::getExportData() const
{
    string path;
    if(m_path.size() > m_exportPathStartIndex)
        path = m_path.substr(m_exportPathStartIndex);


    string buff;
    string pathStr;
    string nameStr;
    string sizeStr;
    string md5Str;
    string lineCountStr;
    string charCountStr;

    if(path.size() > 0)         pathStr = "p<"+path+">";
    if(m_fileName.size() > 0)   nameStr = "n<"+m_fileName+">";
    if(m_size > 0)              sizeStr = "s<"+std::to_string(m_size)+">";
    if(m_md5.size() > 0)        md5Str  = "h<"+m_md5+">";
    if(m_lineCount > 0)         lineCountStr = "l<"+std::to_string(m_lineCount)+">";
    if(m_charCount > 0)         charCountStr = "c<"+std::to_string(m_charCount)+">";

    buff = pathStr + nameStr + sizeStr + md5Str + lineCountStr + charCountStr;
    if(buff.size() > 0)
        return "F{"+buff+"}";
    return "";
}

void FileData::setExportPathBegin(size_t begin)
{
    m_exportPathStartIndex = begin;
}
size_t FileData::getExportPathBegin()
{
    return m_exportPathStartIndex;
}

string FileData::extractValue(string str, const string &keyBegin, const string &keyEnd)
{
    string::size_type begin = str.find(keyBegin);
    if(begin == string::npos)
        return "";

    size_t beginParam = begin+keyBegin.size();
    if(beginParam >= str.size())
        return "";
    str = str.substr(beginParam);

    string::size_type end = str.find(keyEnd);

    if(end == string::npos)
        return str;
    return str.substr(0,end);
}
void FileData::countLines()
{
    string path;
    if(m_path.size()>0)
        path=m_path+"/";
    path+=m_fileName;
    QFile _file(QString::fromStdString(path));
    m_lineCount = 0;
    m_charCount = 0;

    bool slicedHash = true;
    size_t buffLen = 100'000'000;
    if(m_size < buffLen)
        slicedHash = false;
        //buffLen = file.m_size;
    bool finish = false;
    //size_t iteration = 0;
    using namespace std::chrono;


    if(_file.open(QIODevice::ReadOnly))
    {
        if(slicedHash)
        {
            QByteArray fileData(buffLen,0);
            while (!finish)
            {
                //QByteArray fileData = _file.readAll();

                qint64 ret = _file.read(fileData.begin(),buffLen);
                if(ret > 0)
                {
                    countLines(fileData,m_lineCount,m_charCount);
                }
                else
                    finish = true;

                processQEvents();

               // qDebug() << "MD5: "<<file.m_md5.c_str()<<"\t of: "<<file.getPath().c_str();
            }
        }
        else
        {
            QByteArray fileData = _file.readAll();
            countLines(fileData,m_lineCount,m_charCount);
        }
        m_linesCounted = true;
    }
    else
    {
        qDebug() << "Can't open file to count lines. File: "<<path.c_str() << " DATA: "<<getExportData().c_str();
    }
}
void FileData::countLines(QByteArray &arr,uintmax_t &lineC,uintmax_t &charC)
{
    char *c = arr.data();
    char *end = c + arr.size();
    for(char *c = arr.data(); c<end; ++c)
    {
        if(*c=='\n')
        {
            ++lineC;
        } else if(*c>31)
            ++charC;
    }
    ++lineC;
}






#ifdef USE_BOOST_FILESYSTEM
File::File(const boost::filesystem::directory_entry &entry)
    :   m_data(entry)
{
    m_standardItems.reserve(4);
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems[0]->setEditable(false);
    m_standardItems[1]->setEditable(false);
    m_standardItems[2]->setEditable(false);
    m_standardItems[3]->setEditable(false);
    m_standardItems[0]->setText(QString::fromStdString(m_data.getName()));
    m_standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(m_data.getSize())));
    m_standardItems[2]->setText(QString::number(m_data.getSize()));
}
#endif
File::File(const std::filesystem::directory_entry &entry)
    :   m_data(entry)
{
    m_standardItems.reserve(4);
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems[0]->setEditable(false);
    m_standardItems[1]->setEditable(false);
    m_standardItems[2]->setEditable(false);
    m_standardItems[3]->setEditable(false);
    m_standardItems[0]->setText(QString::fromStdString(m_data.getName()));
    m_standardItems[1]->setText(QString::fromStdString(FileData::sizeToStr(m_data.getSize())));
    m_standardItems[2]->setText(QString::number(m_data.getSize()));
}
File::~File()
{

}


FileData &File::getData()
{
    return m_data;
}

const string &File::getName() const
{return m_data.getName();}
const string &File::getPath() const
{return m_data.getPath();}
uintmax_t    File::getSize() const
{return m_data.getSize();}
const string &File::getMd5()
{return m_data.getMd5();}

void File::print(size_t tab)
{
    m_data.print(tab);
}

void File::sortBySize(vector<File*> &list)
{
    std::sort(list.begin(),list.end(), [ ]( const File* lhs, const File* rhs )
    {
       return lhs->m_data.getSize() > rhs->m_data.getSize();
    });
}
bool File::operator > (const File &file) const
{
    return m_data > file.m_data;
}
bool File::operator < (const File &file) const
{
    return m_data < file.m_data;
}
bool File::isEqual(File &other,bool useMd5, bool useName)
{
    return m_data.isEqual(other.m_data,useMd5,useName);
}

QList<QStandardItem *> File::getStandardItem() const
{
    return m_standardItems;
}

const string &File::md5(File& file)
{
    string hash = FileData::md5(file.m_data);
    file.m_standardItems[3]->setText(QString::fromStdString(file.getMd5()));
    return file.getMd5();
}
void File::countLines(File& file)
{
    FileData::countLines(file.m_data);
}

void File::readImportData(const string &data)
{
    m_data.readImportData(data);
}
string File::getExportData()const
{
    return m_data.getExportData();
}


