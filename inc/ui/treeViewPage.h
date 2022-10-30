#ifndef TREEVIEWPAGE_H
#define TREEVIEWPAGE_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHBoxLayout>
#include <string>

class TreeViewPage : public QWidget
{
        Q_OBJECT
    public:
        TreeViewPage(QWidget *parent);
        ~TreeViewPage();

        virtual void setTree();
        virtual void setTree(const QList<QStandardItem*>&tree,
                             const std::vector<std::string> &header);
        void setColumnWidth(size_t column, unsigned int width);
        void clear();
    signals:

    public slots:
        //void show();
        //void hide();

    protected:
        QHBoxLayout *m_layout;
        QTimer *m_updateTimer;
        //QScrollArea *frame;
        QTreeView *m_treeView;
        QStandardItemModel *m_model;
};

#endif // TREEVIEWPAGE_H
