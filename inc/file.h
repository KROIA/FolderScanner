#ifndef FILE_H
#define FILE_H

#include <filesystem>
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <windows.h>

#include <QStandardItem>
#include <QCryptographicHash>
#include <QFile>


using std::string;
using std::vector;

class File
{
    public:
        File(const std::filesystem::directory_entry &entry);
        ~File();

        const string getName() const;
        const string getPath() const;

        uintmax_t size() const;
        const string getMd5();

        void print(size_t tab = 0);

        static void sortBySize(vector<File*> &list);
        bool operator > (const File &file) const;
        bool operator < (const File &file) const;
        bool isEqual(File &other,bool useMd5, bool useName);

        QList<QStandardItem *> getStandardItem() const;
        static string sizeToStr(uintmax_t size);


        static string md5(File& file);

    private:
        std::filesystem::directory_entry m_dirEntry;
        string m_md5;

        QList<QStandardItem*> m_standardItems;
};
#endif // FILE_H
