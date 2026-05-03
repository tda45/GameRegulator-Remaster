#include "mainwindow.h"
#include <QApplication>
#include <QStyle>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("GameRegulator Remaster");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("tda_45");
    app.setOrganizationDomain("github.com/tda45/GameRegulator-Remaster");

    // Set application style
    app.setStyle("Fusion");

    // Load theme settings
    QString userProfile = qgetenv("USERPROFILE");
    QString customAppData = userProfile + "/AppData/Local/GameRegulatorRemasterData";
    QString settingsPath = customAppData + "/settings.ini";
    QString theme = "default"; // default theme
    
    QFile settingsFile(settingsPath);
    if (settingsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&settingsFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("theme=")) {
                theme = line.split("=")[1].trimmed();
                break;
            }
        }
        settingsFile.close();
    }
    
    // Apply theme
    if (theme == "dark") {
        app.setStyleSheet("QMainWindow { background-color: #2b2b2b; } QWidget { background-color: #2b2b2b; color: #ffffff; } QPushButton { background-color: #3c3c3c; color: #ffffff; border: 1px solid #555; } QPushButton:hover { background-color: #4c4c4c; } QListWidget { background-color: #2b2b2b; color: #ffffff; } QMenuBar { background-color: #2b2b2b; color: #ffffff; } QMenu { background-color: #2b2b2b; color: #ffffff; }");
    } else if (theme == "light") {
        app.setStyleSheet("QMainWindow { background-color: #ffffff; } QWidget { background-color: #ffffff; color: #000000; } QPushButton { background-color: #f0f0f0; color: #000000; border: 1px solid #ccc; } QPushButton:hover { background-color: #e0e0e0; } QListWidget { background-color: #ffffff; color: #000000; } QMenuBar { background-color: #ffffff; color: #000000; } QMenu { background-color: #ffffff; color: #000000; }");
    }

    MainWindow window;
    window.show();

    return app.exec();
}
