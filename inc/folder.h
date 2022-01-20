#ifndef FOLDER_H
#define FOLDER_H

#include <filesystem>
#include <string>
#include <vector>
#include <QMutex>
#include <unordered_map>
#include <chrono>

#include "file.h"


using std::string;
using std::vector;
using std::filesystem::directory_iterator;

class Folder
{
    public:
        Folder(Folder *parent = nullptr);
        ~Folder();

        void clear();

        void setName(const string &name);
        const string &getName() const;
        string getPath();

        size_t getMasterFolderCount() const;
        size_t getMasterFileCount() const;
        uintmax_t getMasterContentSize() const;
        double getMasterMD5ScanProgress() const; // percent

        void scan();
        void scanMd5();
        void cancelScan();


        uintmax_t size() const;
        size_t fileCount() const;
        size_t folderCount() const;
        const string getMd5();


        void print(size_t tab = 0);

        static void sortBySize(Folder *owner,vector<Folder*> &list);
        bool operator > (const Folder &folder) const;
        bool operator < (const Folder &folder) const;


        QList<QStandardItem *> getStandardItem() const;
        static bool dirExists(const std::string& dirName_in);

        static string md5(Folder& folder);

    private:
        void incrementFolderSize(uintmax_t size);
        void sort();

        string m_name;
        string m_path;
        Folder *m_parent;
        uintmax_t m_contentSize;
        string m_md5;
        QList<QStandardItem*> m_standardItems;
        bool m_folderIsComplete; // finished scanning


        vector<Folder*> m_childFolderList;
        vector<File*> m_containedFiles;
        //QStandardItem *m_folderNameItem;

        Folder *m_masterFolder;
        void masterAddFolderCount(size_t increment);
        void masterAddFileCount(size_t increment);
        void masterAddContentSize(uintmax_t increment);
        void masterAddMd5Progress(size_t increment);

        size_t *m_master_folderCount;
        size_t *m_master_fileCount;
        uintmax_t *m_master_contentSize;
        uintmax_t *m_master_md5ProgressCounter;
        QMutex *m_masterMutex;
        bool *m_master_escapeScan;
};
#endif // FOLDER_H
