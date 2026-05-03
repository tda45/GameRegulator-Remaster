#include "filemanager.h"
#include <QDirIterator>
#include <QFileInfoList>
#include <QDebug>
#include <QDateTime>
#include <windows.h>
#include <QStandardPaths>
#include <shellapi.h>
#include <QApplication>
#include <QTimer>
#include <QTextStream>

FileManager::FileManager(QObject *parent)
    : QObject(parent)
{
    initializePaths();
}

void FileManager::initializePaths()
{
    // Use OneDrive Desktop path with advanced Windows APIs
    desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    loadFolderSettings();
    gamesFolder = desktopPath + "/" + getGamesFolderName();
}

QString FileManager::getDesktopPath() const
{
    return desktopPath;
}

QString FileManager::getGamesFolderPath() const
{
    return gamesFolder;
}

bool FileManager::createGamesFolder()
{
    logError("DEBUG: Creating games folder. Path: " + gamesFolder);
    logError("DEBUG: Desktop path: " + desktopPath);
    logError("DEBUG: Custom folder name: " + customFolderName);
    
    QDir dir(gamesFolder);
    if (!dir.exists()) {
        logError("DEBUG: Folder does not exist, creating...: " + gamesFolder);
        emit progressUpdated(10, tr("Oyunlar klasörü oluşturuluyor..."));
        
        // Try Windows API CreateDirectory for OneDrive Desktop
        bool created = CreateDirectoryW((LPCWSTR)gamesFolder.utf16(), NULL);
        DWORD error = GetLastError();
        
        if (!created && error != ERROR_ALREADY_EXISTS) {
            // Fallback to Qt mkpath if Windows API fails
            logError("DEBUG: Windows API failed, trying Qt mkpath");
            if (!dir.mkpath(gamesFolder)) {
                QString errorMsg = tr("Oyunlar klasörü oluşturulamadı: ") + gamesFolder;
                logError(errorMsg);
                logError("DEBUG: Failed to create folder: " + gamesFolder);
                emit errorOccurred(errorMsg);
                return false;
            }
        }
        
        logError("DEBUG: Folder created successfully: " + gamesFolder);
        emit progressUpdated(20, tr("Oyunlar klasörü başarıyla oluşturuldu"));
        
        // Force OneDrive Desktop visibility - simple approach
        logError("DEBUG: Using simple OneDrive Desktop approach");
        
        // Set folder to normal attributes
        SetFileAttributesW((LPCWSTR)gamesFolder.utf16(), FILE_ATTRIBUTE_NORMAL);
        logError("DEBUG: Set folder to normal attributes");
        
        // Force desktop refresh without creating extra files
        SendMessageW(FindWindowW(L"Progman", L"Program Manager"), WM_COMMAND, 0xA065, 0);
        logError("DEBUG: Desktop refresh forced - no extra files created");
        
        // Apply folder customization after creation - DISABLED TO AVOID HIDDEN ATTRIBUTES
        // applyFolderCustomization();
    } else {
        logError("DEBUG: Folder already exists: " + gamesFolder);
        
        // Check folder attributes
        QFileInfo folderInfo(gamesFolder);
        DWORD attributes = GetFileAttributesW((LPCWSTR)gamesFolder.utf16());
        logError("DEBUG: Folder attributes: " + QString::number(attributes));
        
        if (attributes & FILE_ATTRIBUTE_HIDDEN) {
            logError("DEBUG: Folder is HIDDEN - removing hidden attribute");
            SetFileAttributesW((LPCWSTR)gamesFolder.utf16(), attributes & ~FILE_ATTRIBUTE_HIDDEN);
        }
        
        if (attributes & FILE_ATTRIBUTE_SYSTEM) {
            logError("DEBUG: Folder is SYSTEM - removing system attribute");
            SetFileAttributesW((LPCWSTR)gamesFolder.utf16(), attributes & ~FILE_ATTRIBUTE_SYSTEM);
        }
        
        // Force desktop refresh - no extra files
        SendMessageW(FindWindowW(L"Progman", L"Program Manager"), WM_COMMAND, 0xA065, 0);
        logError("DEBUG: Desktop refresh forced for existing folder");
    }
    return true;
}

QList<ShortcutInfo> FileManager::findShortcuts(const QString &folderPath)
{
    QList<ShortcutInfo> shortcuts;
    QDir dir(folderPath);
    
    if (!dir.exists()) {
        QString errorMsg = tr("Klasör bulunamadı: ") + folderPath;
        logError(errorMsg);
        emit errorOccurred(errorMsg);
        return shortcuts;
    }

    emit progressUpdated(5, tr("Kısayollar taranıyor..."));
    emit progressUpdated(6, tr("Klasör: ") + folderPath);
    
    // Test with Windows API to see ALL files including hidden ones
    emit progressUpdated(7, tr("Windows API ile tarama..."));
    
    QString searchPath = folderPath;
    if (!searchPath.endsWith("\\")) {
        searchPath += "\\";
    }
    searchPath += "*";
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((LPCWSTR)searchPath.utf16(), &findData);
    
    QStringList windowsFoundFiles;
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // Skip directories
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                QString fileName = QString::fromWCharArray(findData.cFileName);
                windowsFoundFiles << fileName;
                
                // Check file attributes
                QString attrStr = "";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) attrStr += "HIDDEN ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) attrStr += "SYSTEM ";
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) attrStr += "READONLY ";
                
                emit progressUpdated(8, QString(tr("Windows API buldu: %1 - Öznitelikler: %2")).arg(fileName).arg(attrStr));
                
                if (fileName.toLower().endsWith(".lnk")) {
                    QString fullPath = folderPath + "\\" + fileName;
                    
                    // Try to open the file
                    HANDLE hFile = CreateFileW((LPCWSTR)fullPath.utf16(), 
                                             GENERIC_READ, 
                                             FILE_SHARE_READ, 
                                             NULL, 
                                             OPEN_EXISTING, 
                                             FILE_ATTRIBUTE_NORMAL, 
                                             NULL);
                    
                    if (hFile != INVALID_HANDLE_VALUE) {
                        emit progressUpdated(9, QString(tr("LNK dosyası okunabilir: %1")).arg(fileName));
                        CloseHandle(hFile);
                        
                        // Get file creation time
                        QString creationTimeStr = "";
                        if (hFind != INVALID_HANDLE_VALUE) {
                            FILETIME ftCreate = findData.ftCreationTime;
                            
                            // Convert FILETIME to SYSTEMTIME
                            SYSTEMTIME stCreate;
                            FileTimeToSystemTime(&ftCreate, &stCreate);
                            
                            // Format as readable string
                            creationTimeStr = QString("%1/%2/%3 %4:%5:%6")
                                .arg(stCreate.wDay, 2, 10, QChar('0'))
                                .arg(stCreate.wMonth, 2, 10, QChar('0'))
                                .arg(stCreate.wYear)
                                .arg(stCreate.wHour, 2, 10, QChar('0'))
                                .arg(stCreate.wMinute, 2, 10, QChar('0'))
                                .arg(stCreate.wSecond, 2, 10, QChar('0'));
                            
                            emit progressUpdated(9, QString(tr("Oluşturulma: %1")).arg(creationTimeStr));
                        }
                        
                        ShortcutInfo shortcut;
                        shortcut.filePath = fullPath;
                        shortcut.fileName = fileName.left(fileName.length() - 4);
                        shortcut.targetPath = "";
                        shortcut.creationTime = creationTimeStr;
                        shortcut.isSelected = true;
                        shortcuts.append(shortcut);
                        
                        emit progressUpdated(10, tr("Shortcut added: %1 (%2)").arg(shortcut.fileName).arg(creationTimeStr));
                    } else {
                        DWORD error = GetLastError();
                        emit progressUpdated(9, tr("LNK file could not be read: %1 - Error: %2").arg(fileName).arg(error));
                    }
                }
            }
        } while (FindNextFileW(hFind, &findData) != 0);
        
        FindClose(hFind);
    } else {
        DWORD error = GetLastError();
        emit progressUpdated(8, QString(tr("Windows API hata: %1")).arg(error));
    }
    
    emit progressUpdated(20, QString(tr("Windows API ile %1 dosya bulundu")).arg(windowsFoundFiles.size()));
    
    // Also try Qt method for comparison
    emit progressUpdated(25, tr("Qt metodu ile karşılaştırma..."));
    dir.refresh();
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    QStringList qtFiles = dir.entryList();
    emit progressUpdated(26, QString(tr("Qt ile %1 dosya bulundu")).arg(qtFiles.size()));
    
    for (const QString &fileName : qtFiles) {
        if (!windowsFoundFiles.contains(fileName)) {
            emit progressUpdated(27, QString(tr("Qt sadece buldu: %1")).arg(fileName));
        }
    }

    emit progressUpdated(85, QString(tr("%1 kısayol bulundu")).arg(shortcuts.size()));
    return shortcuts;
}

QStringList FileManager::getAllSubdirectories(const QString &rootPath)
{
    QStringList dirs;
    dirs << rootPath;
    
    QDir dir(rootPath);
    dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot);
    
    QStringList subdirs = dir.entryList();
    for (const QString &subdir : subdirs) {
        QString fullPath = dir.absoluteFilePath(subdir);
        dirs << fullPath;
        dirs << getAllSubdirectories(fullPath);
    }
    
    return dirs;
}

bool FileManager::validateShortcut(const QFileInfo &fileInfo) const
{
    // Basic validation - check if file exists and is readable
    if (!fileInfo.exists()) {
        return false;
    }

    if (!fileInfo.isReadable()) {
        return false;
    }

    // Check if it's actually a .lnk file
    if (fileInfo.suffix().toLower() != "lnk") {
        return false;
    }

    return true;
}

bool FileManager::isShortcutFile(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower() == "lnk";
}

bool FileManager::moveSelectedShortcuts(const QList<ShortcutInfo> &shortcuts)
{
    if (!createGamesFolder()) {
        return false;
    }

    emit progressUpdated(90, tr("Kısayollar taşınıyor..."));
    
    int totalShortcuts = shortcuts.size();
    int movedShortcuts = 0;

    for (const auto &shortcut : shortcuts) {
        if (!shortcut.isSelected) {
            continue;
        }

        QString destPath = gamesFolder + "/" + QFileInfo(shortcut.filePath).fileName();
        
        if (moveFile(shortcut.filePath, destPath)) {
            movedShortcuts++;
            int progress = 90 + (movedShortcuts * 10 / totalShortcuts);
            emit progressUpdated(progress, QString(tr("Taşınıyor: %1")).arg(shortcut.fileName));
        } else {
            QString errorMsg = tr("Kısayol taşınamadı: ") + shortcut.fileName;
            logError(errorMsg);
            emit errorOccurred(errorMsg);
        }
    }

    emit progressUpdated(100, QString(tr("%1 kısayol başarıyla taşındı")).arg(movedShortcuts));
    emit operationCompleted(true, QString(tr("%1 kısayol Oyunlar klasörüne taşındı")).arg(movedShortcuts));
    
    return true;
}

bool FileManager::fileExists(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists();
}

bool FileManager::createDirectory(const QString &dirPath) const
{
    QDir dir(dirPath);
    return dir.mkpath(dirPath);
}

bool FileManager::moveFile(const QString &sourcePath, const QString &destPath) const
{
    QFile file(sourcePath);
    
    // Check if source exists
    if (!file.exists()) {
        QString errorMsg = "Source file does not exist: " + sourcePath;
        logError(errorMsg);
        qDebug() << errorMsg;
        return false;
    }
    
    // If destination already exists, generate a unique name
    QString finalDestPath = destPath;
    int counter = 1;
    while (QFile::exists(finalDestPath)) {
        QFileInfo fileInfo(destPath);
        QString baseName = fileInfo.baseName();
        QString suffix = fileInfo.completeSuffix();
        finalDestPath = fileInfo.absolutePath() + "/" + baseName + QString(" (%1)").arg(counter) + "." + suffix;
        counter++;
    }
    
    qDebug() << "Copying from:" << sourcePath << "to:" << finalDestPath;
    
    // Copy file first
    bool success = file.copy(finalDestPath);
    
    if (!success) {
        QString errorMsg = "Copy failed for " + sourcePath + " to " + finalDestPath + ": " + file.errorString();
        logError(errorMsg);
        qDebug() << errorMsg;
        return false;
    }
    
    // If copy successful, delete source file to complete move operation
    QFile sourceFile(sourcePath);
    if (!sourceFile.remove()) {
        QString errorMsg = "Failed to delete source file after copy: " + sourcePath + " - " + sourceFile.errorString();
        logError(errorMsg);
        qDebug() << errorMsg;
        // Copy was successful but delete failed - still consider it a partial success
        return false;
    }
    
    qDebug() << "Move successful: copied and deleted source";
    return true;
}

void FileManager::logError(const QString &error) const
{
    QString userProfile = qgetenv("USERPROFILE");
    QString customAppData = userProfile + "/AppData/Local/GameRegulatorRemasterData";
    QString logPath = customAppData + "/errorlogs.log";
    
    QDir().mkpath(customAppData);
    
    QFile logFile(logPath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "] ";
        out << "ERROR: " << error << "\n";
        logFile.close();
    }
}

void FileManager::loadFolderSettings()
{
    QString userProfile = qgetenv("USERPROFILE");
    QString customAppData = userProfile + "/AppData/Local/GameRegulatorRemasterData";
    QString settingsPath = customAppData + "/settings.ini";
    
    // Default values
    customFolderName = "Oyunlar";
    customFolderColor = "yellow";
    customFolderIcon = "";
    
    QFile settingsFile(settingsPath);
    if (settingsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&settingsFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("folder_name=")) {
                customFolderName = line.split("=")[1].trimmed();
            } else if (line.startsWith("folder_color=")) {
                customFolderColor = line.split("=")[1].trimmed();
            } else if (line.startsWith("folder_icon=")) {
                customFolderIcon = line.split("=")[1].trimmed();
            }
        }
        settingsFile.close();
    }
}

void FileManager::saveFolderSettings()
{
    QString userProfile = qgetenv("USERPROFILE");
    QString customAppData = userProfile + "/AppData/Local/GameRegulatorRemasterData";
    QString settingsPath = customAppData + "/settings.ini";
    
    QDir().mkpath(customAppData);
    
    QFile settingsFile(settingsPath);
    if (settingsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&settingsFile);
        out << "folder_name=" << customFolderName << "\n";
        out << "folder_color=" << customFolderColor << "\n";
        out << "folder_icon=" << customFolderIcon << "\n";
        settingsFile.close();
    }
}

QString FileManager::getGamesFolderName() const
{
    return customFolderName.isEmpty() ? "Oyunlar" : customFolderName;
}

void FileManager::applyFolderCustomization()
{
    // DISABLED: Only name changes are allowed now
    // Color and icon settings are disabled to avoid hidden attributes
    logError("DEBUG: Folder customization disabled - only name changes allowed");
}
