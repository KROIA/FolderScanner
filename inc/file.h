#ifndef FILE_H
#define FILE_H

//#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <windows.h>

#include <QApplication>
#include <QStandardItem>
#include <QCryptographicHash>
#include <QFile>

#include "utility.h"
#include "stringUtility.h"

//#define USE_BOOST_FILESYSTEM

#ifdef USE_BOOST_FILESYSTEM
#include <boost/filesystem.hpp>
#endif

using std::string;
using std::vector;

class FileData
{
    public:
        FileData();
#ifdef USE_BOOST_FILESYSTEM
        FileData(const boost::filesystem::directory_entry &entry);
#endif
        FileData(const std::filesystem::directory_entry &entry);
        FileData(const FileData &other);
        ~FileData();

        void setName(const string &name);
        const string &getName() const; // name with file extention
        string getExtention() const;
        void setPath(const string &path);
        const string &getPath() const;

        void setSize(uintmax_t size);
        uintmax_t getSize() const;
        void setLineCount(uintmax_t lineCount);
        uintmax_t getLineCount();
        void setCharCount(uintmax_t charCount);
        uintmax_t getCharCount();

        void setMd5(const string &md5);
        const string &getMd5();

        void print(size_t tab = 0);

        static void sortBySize(vector<FileData*> &list);
        bool operator > (const FileData &file) const;
        bool operator < (const FileData &file) const;
        bool isEqual(FileData &other,bool useMd5, bool useName);

        static string sizeToStr(uintmax_t size);


        static const string &md5(FileData& file);
        static void countLines(FileData& file);

        bool readImportData(string data);
        static FileData buildFromImport(const string &data);
        static vector<FileData> buildFromImport(const vector<string> &dataList);
        string getExportData() const;

        static void setExportPathBegin(size_t begin);
        static size_t getExportPathBegin();
    private:
        string extractValue(string str, const string &keyBegin, const string &keyEnd);
        void countLines();
        void countLines(QByteArray &arr,uintmax_t &lineC,uintmax_t &charC);

        static size_t m_exportPathStartIndex; // For export, index on which the Path will be displayed
        string m_md5;
        string m_fileName;
        string m_path;
        uintmax_t m_size;

        bool m_linesCounted;
        uintmax_t m_lineCount;
        uintmax_t m_charCount;
};

class File
{
    public:
#ifdef USE_BOOST_FILESYSTEM
        File(const boost::filesystem::directory_entry &entry);
#endif
        File(const std::filesystem::directory_entry &entry);
        ~File();

        FileData &getData();
        const string &getName() const;
        const string &getPath() const;
        uintmax_t getSize() const;
        const string &getMd5();

        void print(size_t tab = 0);

        static void sortBySize(vector<File*> &list);
        bool operator > (const File &file) const;
        bool operator < (const File &file) const;
        bool isEqual(File &other,bool useMd5, bool useName);

        QList<QStandardItem *> getStandardItem() const;


        static const string &md5(File& file);
        static void countLines(File& file);

        void readImportData(const string &data);
        string getExportData()const;

    private:
        //std::filesystem::directory_entry m_dirEntry;
        FileData m_data;

        QList<QStandardItem*> m_standardItems;
};
#endif // FILE_H
