#include "treeViewPage.h"


TreeViewPage::TreeViewPage(QWidget *parent) :
    QWidget(parent)
{
    m_layout = new QHBoxLayout;
    m_treeView = new QTreeView(this);
    m_layout->addWidget(m_treeView);
    parent->setLayout(m_layout);
    m_model = new QStandardItemModel(this);
}
TreeViewPage::~TreeViewPage()
{
    delete m_layout;
}

void TreeViewPage::setTree()
{
    qDebug() << __FILE__<< " "<<__LINE__<< " "<< __PRETTY_FUNCTION__ << " not implemented";
}
void TreeViewPage::setTree(const QList<QStandardItem*>&tree,
                     const std::vector<std::string> &header)
{
    QStandardItem *item = m_model->invisibleRootItem();;

    if(item->rowCount() == 0)
    {
        item->appendRow(tree);
        for(size_t i=0; i<header.size(); ++i)
        {
            m_model->setHeaderData(i,Qt::Horizontal, header[i].c_str());
        }

        m_treeView->setModel(m_model);
        setColumnWidth(0,300);

        if(item->hasChildren())
        {
            m_treeView->expand(item->child(0,0)->index());
        }
    }
}
void TreeViewPage::setColumnWidth(size_t column, unsigned int width)
{
    m_treeView->setColumnWidth(column,width);
}
void TreeViewPage::clear()
{
    m_model->clear();
}
