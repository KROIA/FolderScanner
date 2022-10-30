#ifndef FILESCANPAGE_H
#define FILESCANPAGE_H

#include <QWidget>

#include "treeViewPage.h"
#include "scanThread.h"
#include "stringUtility.h"


namespace Ui {
class FileScanPage;
}

class FileScanPage : public QWidget
{
        Q_OBJECT

    public:
        explicit FileScanPage(ScanThread* scanThread,QWidget *parent = nullptr);
        ~FileScanPage();

        void scanFiles();
        void scanMD5();
    signals:
        void setProgress(double percent,const string &message = "");

    public slots:
        void updateTree();
        void clear();

    private slots:
        void onUpdateTimer();
    private:


        Ui::FileScanPage *ui;
        TreeViewPage *m_treeView;
        ScanThread *m_scanThread;
        QTimer *m_updateTimer;


};

#endif // FILESCANPAGE_H
