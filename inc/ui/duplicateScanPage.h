#ifndef DUPLICATESCANPAGE_H
#define DUPLICATESCANPAGE_H

#include <QWidget>

#include "treeViewPage.h"
#include "scanThread.h"

namespace Ui {
class DuplicateScanPage;
}


class DuplicateScanPage : public QWidget
{
        Q_OBJECT
        enum ProgressStep
        {
            none,
            matchDuplicates,
            buildTreeView
        };
    public:
        explicit DuplicateScanPage(ScanThread* scanThread,QWidget *parent = nullptr);
        ~DuplicateScanPage();



    signals:
        void setProgress(double percent,const string &message = "");
    public slots:
        void scan();
        void deleteDuplicates();
        void clear();

    private slots:
        void onUpdateTimer();


    private:
        void updateProgress();
        void searchDuplicates(vector<FileData> &data);

        Ui::DuplicateScanPage *ui;

        TreeViewPage *m_treeView;
        ScanThread *m_scanThread;

        QTimer *m_updateTimer;
        double m_progressPercent;
        ProgressStep m_progressStep;

        std::unordered_map<std::string, vector<FileData>> m_duplicates;
        vector<FileData> m_content;


};

#endif // DUPLICATESCANPAGE_H
