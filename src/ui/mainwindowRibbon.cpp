#include "mainwindowRibbon.h"

const string MainWindowRibbon::m_externIconsPath = "icons";
const string MainWindowRibbon::m_externIconsExtention = ".png";
const string MainWindowRibbon::m_qIconBasePath = ":/"+m_externIconsPath+"/";
MainWindowRibbon::MainWindowRibbon(Ribbon* ribbonWidget)
    : QWidget(nullptr)
{
    m_ribbon = ribbonWidget;
    buildButtons();

    // Add tabs to ribbon

    //m_ribbon->addTab(QIcon((m_qIconBasePath+"monitor_1"+m_externIconsExtention).c_str()), "View");

    //m_ribbon->addTab(QIcon((m_qIconBasePath+"information_1"+m_externIconsExtention).c_str()), "Help");




    /*

    // Add 'Open project' button
    QToolButton *openProjectButton = new QToolButton;
    openProjectButton->setText(tr("Open"));
    openProjectButton->setToolTip(tr("Open existing project"));
    openProjectButton->setIcon(QIcon(qIconBasePath+"live_folder_2.png"));
    openProjectButton->setEnabled(true);
    m_ribbon->addButton("Project", "Project", openProjectButton);

    // Add 'New project' button
    QToolButton *newProjectButton = new QToolButton;
    newProjectButton->setText(tr("New"));
    newProjectButton->setToolTip(tr("Create new project"));
    newProjectButton->setIcon(QIcon(qIconBasePath+"create_new_2.png"));
    newProjectButton->setEnabled(true);
    m_ribbon->addButton("Project", "Project", newProjectButton);

    // Add 'Save project' button
    QToolButton *saveProjectButton = new QToolButton;
    saveProjectButton->setText(tr("Save"));
    saveProjectButton->setToolTip(tr("Save project"));
    saveProjectButton->setIcon(QIcon(qIconBasePath+"save_2.png"));
    saveProjectButton->setEnabled(false);
    m_ribbon->addButton("Project", "Project", saveProjectButton);

    // Add 'Open file' button
    QToolButton *openFileButton = new QToolButton(this);
    openFileButton->setText(tr("File"));
    openFileButton->setToolTip(tr("Open file or directory"));
    openFileButton->setIcon(QIcon(qIconBasePath+"add_folder_2.png"));

    // Add dropdown menu to button
    openFileButton->setPopupMode(QToolButton::MenuButtonPopup);
    QMenu *menu = new QMenu("Title");
    //menu->addAction(QIcon(":/icons/folder_2.png"),
    //                "Recent directory");
    menu->addAction(QIcon(qIconBasePath+"file_2.png"), "Recent file 1");
    menu->addAction(QIcon(qIconBasePath+"file_2.png"), "Recent file 2");
    menu->addAction(QIcon(qIconBasePath+"file_2.png"), "Recent file 3");
    openFileButton->setMenu(menu);

    m_ribbon->addButton("Project", "Import", openFileButton);

    // Add 'Open database' button
    QToolButton *openDatabaseButton = new QToolButton;
    openDatabaseButton->setText(tr("Database"));
    openDatabaseButton->setToolTip(tr("Connect to database"));
    openDatabaseButton->setIcon(QIcon(qIconBasePath+"add_database_2.png"));
    m_ribbon->addButton("Project", "Import", openDatabaseButton);


    // Add 'Connect to web service' button
    QToolButton *connectWebserviceButton = new QToolButton;
    connectWebserviceButton->setText(tr("Web service"));
    connectWebserviceButton->setToolTip(tr("Connect to web service"));
    connectWebserviceButton->setIcon(QIcon(qIconBasePath+"add_link_2.png"));
    m_ribbon->addButton("Project", "Import", connectWebserviceButton);*/
}
MainWindowRibbon::~MainWindowRibbon()
{

}
ProjectButtons MainWindowRibbon::getProjectButtons() const
{
    return m_projectButtons;
}
ToolButtons    MainWindowRibbon::getToolButtons() const
{
    return m_toolButtons;
}

QToolButton *MainWindowRibbon::buttonFactory(const string &text, const string &toolTip,const string &iconName,
                           bool enabled,const string &tab,const string &group)
{
    QToolButton *button = new QToolButton;
    button->setText(tr(text.c_str()));
    button->setToolTip(tr(toolTip.c_str()));
    button->setIcon(QIcon((m_qIconBasePath+iconName+m_externIconsExtention).c_str()));
    m_ribbon->addButton(tab.c_str(), group.c_str(), button);
    button->setEnabled(enabled);
    return button;
}
void MainWindowRibbon::buildButtons()
{
    buildProjectButtons();
    buildToolButtons();
}
void MainWindowRibbon::buildProjectButtons()
{
    string tab = "Project";
    m_ribbon->addTab(QIcon((m_qIconBasePath+"briefcase_1"+m_externIconsExtention).c_str()), tab.c_str());
    m_projectButtons.loadPathButton   = buttonFactory("Load path","Path which will be scanned",
                                                    "live_folder_2", true, tab, tab);

    m_projectButtons.saveResultButton = buttonFactory("Save result","Save the scanned files, so you can load it to compare to a other path",
                                                    "save_2", true, tab, tab);

    m_projectButtons.scanFolderButton = buttonFactory("Scan path","Scanning for all folders and files in the selected path",
                                                    "scanArchive_1", true, tab, tab);
    m_projectButtons.scanMd5Button    = buttonFactory("Scan MD5-Hash","Creates a MD5-Hash for every file and folder to be able to compare different files and folder structures",
                                                    "md5Scan", true, tab, tab);

    m_projectButtons.cancelScanButton    = buttonFactory("Cancel scan","Cancels the current scan job",
                                                    "cancel", true, tab, tab);
}
void MainWindowRibbon::buildToolButtons()
{
    string tab = "Tools";
    m_ribbon->addTab(QIcon((m_qIconBasePath+"engineering_1"+m_externIconsExtention).c_str()), tab.c_str());
    //m_toolButtons.compareLoadFirstFileButton = buttonFactory("Load to compare","Loads a exported result to compare the files to the current directory",
    //                                             "file_2", true, tab, "Compare to other folder");
    m_toolButtons.compareCurrentToLoadedFile = buttonFactory("Compare","Loads a exported result to compare the files to the current directory",
                                                 "fileCompare", true, tab, "Compare to other folder");

    m_toolButtons.searchForDuplicates        = buttonFactory("Search for duplicates","Searches for duplicate files in the current directory",
                                                 "equalSign", true, tab, "Analyse this directory");
    m_toolButtons.countFileContent           = buttonFactory("Count lines","Read all files and count the lines for every file",
                                                 "counter", true, tab, "Analyse this directory");
}
