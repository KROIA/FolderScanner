#include "file.h"


File::File(const std::filesystem::directory_entry &entry)
{
    m_dirEntry = entry;

    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems.push_back(new QStandardItem());
    m_standardItems[0]->setEditable(false);
    m_standardItems[1]->setEditable(false);
    m_standardItems[2]->setEditable(false);
    m_standardItems[3]->setEditable(false);
    m_standardItems[0]->setText(QString::fromStdString(m_dirEntry.path().filename().string()));
    m_standardItems[1]->setText(QString::fromStdString(File::sizeToStr(size())));
    m_standardItems[2]->setText(QString::number(size()));
}
File::~File()
{

}


const string File::getName() const
{
    return m_dirEntry.path().filename().string();
}
const string File::getPath() const
{
    return m_dirEntry.path().string();
}

uintmax_t File::size() const
{
    return m_dirEntry.file_size();
}
const string File::getMd5()
{
    if(m_md5 == "")
        return md5(*this);
    return m_md5;
}

void File::print(size_t tab)
{
    string tabs;
    for(size_t i=0; i<tab; ++i)
        tabs+="  ";
    std::cout <<tabs<<getName()<<" "<<size()<<" bytes\n";
}

void File::sortBySize(vector<File*> &list)
{
    std::sort(list.begin(),list.end(), [ ]( const File* lhs, const File* rhs )
    {
       return lhs->size() > rhs->size();
    });
}
bool File::operator > (const File &file) const
{
    return size() > file.size();
}
bool File::operator < (const File &file) const
{
    return size() < file.size();
}
bool File::isEqual(File &other,bool useMd5, bool useName)
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

QList<QStandardItem *> File::getStandardItem() const
{
    return m_standardItems;
   /* return {new QStandardItem(QString::fromStdString(getName())),
            new QStandardItem(QString::fromStdString(sizeToStr(size()))),
            new QStandardItem(QString::number(size())),
            new QStandardItem(QString::fromStdString(m_md5))};*/
}
string File::sizeToStr(uintmax_t size)
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

string File::md5 (File& file) {

    QFile _file(QString::fromStdString(file.getPath()));
    if (_file.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = _file.readAll();

        QByteArray hashData = QCryptographicHash::hash(fileData,QCryptographicHash::Md5); // or QCryptographicHash::Sha1
        file.m_md5 = hashData.toHex().toStdString();
        file.m_standardItems[3]->setText(QString::fromStdString(file.m_md5));

       // qDebug() << "MD5: "<<file.m_md5.c_str()<<"\t of: "<<file.getPath().c_str();
    }
    else
    {
        qDebug() << "Can't open file to calculate the MD5 hash. File: "<<file.getPath().c_str();
    }
    return file.m_md5;
}


