#pragma once

#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QCheckBox>
#include <QLayout>

#include "folder.h"
#include "stringUtility.h"
#include "treeViewPage.h"
#include "scanThread.h"
#include "utility.h"

namespace Ui {
class LineCountPage;
}
struct ScanFilter
{
    // If the name in a filter is found at any positions as folder name
    // the filter gets a match
    //vector<string> dirWhiteList; // names of folders, not the whole path
    vector<string> dirBlackList; // names of folders, not the whole path

    vector<string> fileExtentionWhiteList;
    vector<string> fileExtentionBlackList;

    // force the whitelist even if the list is empty
    bool force_fileExtentionWhiteList;
};



class LineCountPage : public QWidget
{
        Q_OBJECT
        enum ProgressStep
        {
            none,
            readingFiles,
            filtering,
            buildTableView,
            buildTreeView
        };

    public:
        explicit LineCountPage(ScanThread* scanThread,QWidget *parent = nullptr);
        ~LineCountPage();

        void setScanFilter(const ScanFilter &filter);
        const ScanFilter &getScanFilter();

        void scan();
    signals:
        void setProgress(double percent,const string &message = "");

    public slots:
        //void updateTree();
        void clear();

    private slots:
        void onFilterCheckBoxClicked(bool state);
        void on_applyFilter_pushButton_clicked();
        void on_filterEnableNone_pushButton_clicked();
        void on_filterEnableAll_pushButton_clicked();
        void on_filterSearch_lineEdit_textChanged(const QString &arg1);
        void onUpdateTimer();







    private:
        void sortFilterCheckBoxes(vector<QCheckBox*> &list);
        void updateScannedFiles();
        void updateProgress();
        void buildTable(Folder *mainFolder);
        void buildTree(Folder *mainFolder);
        void buildTreeRecursive(Folder *folder, QList<QStandardItem*> &tree, uintmax_t &bytes, uintmax_t &lineCount, uintmax_t &charCount);

        static void getFiltered(const vector<FileData> &rawIn,
                                vector<FileData> &out,
                                const ScanFilter &filter);
        static bool isInFilter(const FileData &file, const ScanFilter &filter);
        static bool isInFilter(const string &path,const string &fileName,
                               //const vector<string> &dirWhiteList,
                               const vector<string> &dirBlackList,
                               const vector<string> &fileExtentionWhiteList,
                               const vector<string> &fileExtentionBlackList,
                               bool force_fileExtentionWhiteList);

        void updateFileExtentionCheckBoxFilter();
        void updateFilter();
        void getAllFileExtentions(vector<string> &ext);
        Ui::LineCountPage *ui;

        QWidget *m_parent;
        QStandardItemModel *m_tableModel;
        TreeViewPage *m_treeView;
        ScanThread *m_scanThread;
        //QTableView *m_tableView;
        ScanFilter m_scanFilter;
        static const string m_noExtentionStr;

        std::unordered_map<string,QCheckBox*> m_extentionFilterCheckBoxList;


        QTimer *m_updateTimer;
        //string m_progressText;
        double m_progressPercent;
        ProgressStep m_progressStep;
        size_t m_treeFolderCounter;
        size_t m_treeFolderCounterMax;

};

