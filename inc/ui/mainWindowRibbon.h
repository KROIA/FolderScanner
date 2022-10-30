#ifndef MAINWINDOWRIBBON_H
#define MAINWINDOWRIBBON_H

#include <QMenu>
#include <string>
#include "ribbon.h"

using std::string;
struct ProjectButtons
{
    QToolButton *loadPathButton;
    QToolButton *saveResultButton;

    QToolButton *scanFolderButton;
    QToolButton *scanMd5Button;

    QToolButton *cancelScanButton;
};
struct ToolButtons
{
    //QToolButton *compareLoadFirstFileButton;
    //QToolButton *compareLoadSecondFileButton;

    //QToolButton *compareLoadedFilesButton;
    QToolButton *compareCurrentToLoadedFile;

    QToolButton *searchForDuplicates;
    QToolButton *countFileContent;
};

class MainWindowRibbon : public QWidget
{
        Q_OBJECT
    public:
        MainWindowRibbon(Ribbon* ribbonWidget);
        ~MainWindowRibbon();

        ProjectButtons getProjectButtons() const;
        ToolButtons    getToolButtons() const;
    private:
        QToolButton *buttonFactory(const string &text, const string &toolTip,const string &iconName,
                                   bool enabled,const string &tab,const string &group);
        void buildButtons();
        void buildProjectButtons();
        void buildToolButtons();


        Ribbon *m_ribbon;
        static const string m_externIconsPath;
        static const string m_externIconsExtention;
        static const string m_qIconBasePath;




        ProjectButtons m_projectButtons;
        ToolButtons m_toolButtons;
};

#endif // MAINWINDOWRIBBON_H
