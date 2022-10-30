#ifndef COMPARESCANPAGE_H
#define COMPARESCANPAGE_H

#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTableView>
#include <QTimer>

#include "file.h"
#include "stringUtility.h"

namespace Ui {
class CompareScanPage;
}

class CompareScanPage : public QWidget
{
        Q_OBJECT
        enum ProgressStep
        {
            none,
            comparing,
            buildTableView
        };
    public:
        explicit CompareScanPage(QWidget *parent = nullptr);
        ~CompareScanPage();


        void compare(vector<FileData> &A,
                     vector<FileData> &B);
    signals:
        void setProgress(double percent,const string &message = "");
    public slots:
        //void updateTree();
        void clear();

    private slots:
        void onUpdateTimer();
    private:
        void updateProgress();
        Ui::CompareScanPage *ui;

        QWidget *m_parent;
        QStandardItemModel *m_tableModel;
        //QTableView *m_tableView;

        QTimer *m_updateTimer;
        double m_progressPercent;
        ProgressStep m_progressStep;

};

#endif // COMPARESCANPAGE_H
