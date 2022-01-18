#ifndef FOLDER_H
#define FOLDER_H

#include <filesystem>
#include <string>
#include <vector>
#include <QMutex>

#include "file.h"


using std::string;
using std::vector;
using std::filesystem::directory_iterator;

class Folder
{
    public:
        Folder(Folder *parent = nullptr);
        ~Folder();

        void setName(const string &name);
        const string &getName() const;
        string getPath();

        size_t getMasterFolderCount() const;
        size_t getMasterFileCount() const;
        uintmax_t getMasterContentSize() const;

        void scan();
        void cancelScan();

        uintmax_t size() const;
        size_t fileCount() const;
        size_t folderCount() const;

        void print(size_t tab = 0);

        static void sortBySize(vector<Folder*> &list);
        bool operator > (const Folder &folder) const;
        bool operator < (const Folder &folder) const;

        QList<QStandardItem *> getStandardItem() const;
        static bool dirExists(const std::string& dirName_in);

    private:
        string m_name;
        string m_path;
        Folder *m_parent;
        uintmax_t m_contentSize;
        //bool m_folderIsComplete; // finished scanning


        vector<Folder*> m_childFolderList;
        vector<File*> m_containedFiles;

        Folder *m_masterFolder;
        void masterAddFolderCount(size_t increment);
        void masterAddFileCount(size_t increment);
        void masterAddContentSize(uintmax_t increment);

        size_t *m_master_folderCount;
        size_t *m_master_fileCount;
        uintmax_t *m_master_contentSize;
        QMutex *m_masterMutex;
        bool *m_master_escapeScan;
};
#endif // FOLDER_H
