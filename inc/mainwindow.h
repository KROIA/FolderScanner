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

#include "folder.h"
#include "scanThread.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        void scanFolder(const std::string &folderPath);


        void updateUIstats();
        void updateUITreeView();

    private slots:
        void on_scan_pushButton_clicked();
        void onScanUpdateTimer();
        void onScanMd5UpdateTimer();

        void onScanThreadResultFinish(ScanThread *thread, const Folder &folder);

        void on_cancel_pushButton_clicked();

        void on_scanMd5_pushButton_clicked();

        void on_rescan_pushButton_clicked();

    private:
        Ui::MainWindow *ui;
        QTimer *scanUpdateTimer;
        QTimer *scanMd5UpdateTimer;
        ScanThread *scanThread;

        QScrollArea *frame;
        QTreeView *treeView;
        QStandardItemModel *standardModel;
};
#endif // MAINWINDOW_H
