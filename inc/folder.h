#ifndef FOLDER_H
#define FOLDER_H

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <QMutex>
#include <unordered_map>
#include <chrono>
#include <locale>
#include <codecvt>

#include <thread>         // std::thread
#include <mutex>

//#include <boost/filesystem.hpp>

#include "file.h"

#define FOLDER_TRHEADED_SCAN


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
        double getMasterProgress() const; // percent

        void scanThreaded();
        void scan();
        void scanMd5();
        void resetProgress();
        void cancelScan();



        uintmax_t size() const;
        size_t fileCount() const;
        size_t folderCount() const;
        const vector<Folder*> &getSubFolder();
        const vector<File*> &getFiles();
        const string getMd5();


        void print(size_t tab = 0);

        static void sortBySize(Folder *owner,vector<Folder*> &list);
        bool operator > (const Folder &folder) const;
        bool operator < (const Folder &folder) const;


        QList<QStandardItem *> getStandardItem() const;
        static bool dirExists(const std::string& dirName_in);

        static string md5(Folder& folder);

       // void readImportData(const vector<string> &data);
       // vector<string> getExportData();
        bool importFromFile(const string &file);
        bool exportToFile(const string &file) const;
        void exportToTable(vector<string> &table) const;
        void getAllFileDataRecursive(vector<FileData> &list) const;
        void getAllFilesRecursive(vector<File*> &list) const;
        void getAllFoldersRecursive(vector<Folder*> &list);

    private:
        void scanInternal();
        void internalImportFromFile(std::ifstream &s);
        void internalExportToFile(std::ofstream &s)const;
        void internalExportToTable(vector<string> &table)const;
        void internalGetAllFilesRecursive(vector<FileData> &list) const;
        void internalGetAllFilesRecursive(vector<File*> &list) const;
        void internalGetAllFoldersRecursive(vector<Folder*> &list) const;
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
    public:
        void masterResetProgress();
        void masterAddProgress(size_t increment);
    private:
        size_t *m_master_folderCount;
        size_t *m_master_fileCount;
        uintmax_t *m_master_contentSize;
        uintmax_t *m_master_progressCounter;
        QMutex *m_masterMutex;
        bool *m_master_escapeScan;
#ifdef FOLDER_TRHEADED_SCAN
        struct ThreadData
        {
            size_t threadIndex;
            Folder **list;
            size_t listSize;
            size_t *beginPoint;
            size_t jobSize;
            bool *exit;
            int *threadsActiveCount;
        };

        static std::mutex m_mutex;
        static int m_threadActiveCount;
        static bool m_threadExit;
        static size_t m_maxThreadCount;
        static vector<std::thread*> m_threadList;
        static bool m_threadsExecuted;


        static void scanThreadFunc(ThreadData *data);

#endif
};
#endif // FOLDER_H
