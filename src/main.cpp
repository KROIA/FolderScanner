#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "mainwindow.h"

#include <QApplication>
#include <QStyleFactory>
#include <QtPlugin>

#include "folder.h"

using std::cout; using std::cin;
using std::endl; using std::string;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Q_IMPORT_PLUGIN(QSvgPlugin)
    qDebug() << QStyleFactory::keys();
    a.setStyle(QStyleFactory::create("Fusion"));
   /* string path = "E:\\Dokumente\\QT\\Projects\\FolderScanner";

    Folder folder;
    folder.setName(path);
    folder.scan();

    folder.print();*/
        /*for (const auto & file : directory_iterator(path))
        {
            cout << file.path() << "\t";
            cout << file.is_directory() << endl;
            //file.file_size()



        }*/

    MainWindow w;
    w.show();
    return a.exec();
}
