#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QScrollArea>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>

#include <unordered_map>

#include "folder.h"
#include "scanThread.h"
#include "stringUtility.h"
#include "fileScanPage.h"
#include "duplicateScanPage.h"
#include "compareScanPage.h"
#include "lineCountPage.h"
#include "mainwindowRibbon.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        //void scanFolder(const std::string &folderPath);


    private slots:
        void onLoadPath();
        void on_scan_pushButton_clicked();
        void on_scanMd5_pushButton_clicked();
        void on_rescan_pushButton_clicked();
        void on_cancel_pushButton_clicked();
        void onThreadTaskUpdateTimer();
        void onScanThreadResultFinish(ScanThread *thread, const Folder &folder);

        void onExportButtonClicked();
        void onCompareButtonClicked();

        void on_scanDuplicates_pushButton_clicked();
        void onCountLinesButtonClicked();

        void onVersionButtonClicked();

        void on_path_lineEdit_textChanged(const QString &arg1);

        void setProgress(double percent,const string &message = "");
        //void setProgress(double percent);
    private:

        //bool compareTables(vector<FileData> &A,
        //                   vector<FileData> &B);
        //void searchDuplicates(vector<FileData> &data);




        Ui::MainWindow *ui;
        static const string m_versionsText;
        MainWindowRibbon *m_ribbon;
        //QTimer *threadTaskUpdateTimer;
        ScanThread *scanThread;

        QStandardItemModel *tableModel;

        enum PageIndex
        {
            fileScanPage,
            compareScanPage,
            duplicateScanPage,
            lineCountPage
        };
        void goToPage(PageIndex index);

        FileScanPage *m_fileScanTreeView;
        CompareScanPage *m_compareScanPage;
        DuplicateScanPage *m_duplicateScanTreeView;
        LineCountPage *m_lineCountPage;

        bool m_scanNeedsUpdate;
        bool m_md5NeedsUpdate;

};
#endif // MAINWINDOW_H
