#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

struct ShortcutInfo {
    QString filePath;
    QString fileName;
    QString targetPath;
    QString creationTime;
    bool isSelected;
};

class FileManager : public QObject
{
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    
    // Folder operations
    bool createGamesFolder();
    QString getGamesFolderPath() const;
    QString getDesktopPath() const;
    
    // Shortcut operations
    QList<ShortcutInfo> findShortcuts(const QString &folderPath);
    bool moveSelectedShortcuts(const QList<ShortcutInfo> &shortcuts);
    
    // File system operations
    bool fileExists(const QString &filePath) const;
    bool createDirectory(const QString &dirPath) const;
    bool moveFile(const QString &sourcePath, const QString &destPath) const;
    
    // Utility functions
    bool isShortcutFile(const QString &filePath) const;

signals:
    void progressUpdated(int value, const QString &message);
    void operationCompleted(bool success, const QString &message);
    void errorOccurred(const QString &error);

private:
    QString gamesFolder;
    QString desktopPath;
    QString customFolderName;
    QString customFolderColor;
    QString customFolderIcon;
    
    void initializePaths();
    bool validateShortcut(const QFileInfo &fileInfo) const;
    QStringList getAllSubdirectories(const QString &rootPath);
    void logError(const QString &error) const;
    void loadFolderSettings();
    void saveFolderSettings();
    QString getGamesFolderName() const;
    void applyFolderCustomization();
};

#endif // FILEMANAGER_H
