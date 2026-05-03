#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QCheckBox>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfoList>
#include <QTimer>
#include <QLineEdit>
#include <QComboBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>
#include <QGridLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include "filemanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectSourceFolder();
    void organizeShortcuts();
    void onShortcutSelectionChanged();
    void updateProgress(int value, const QString &message);
    void showAboutDialog();
    void showSettingsDialog();
    void filterShortcuts(const QString &text);
    void filterByCategory(const QString &category);
    void backupShortcuts();
    void restoreShortcuts();
    void showStatistics();
    void refreshShortcutList();
    void showHelpDialog();

private:
    void setupUI();
    void connectSignals();
    void scanForShortcuts(const QString &folderPath);
    void createGamesFolder();
    void resetUI();

    // UI Components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *searchLayout;
    QTabWidget *tabWidget;
    
    QPushButton *selectFolderButton;
    QPushButton *organizeButton;
    QPushButton *backupButton;
    QPushButton *restoreButton;
    QPushButton *statisticsButton;
    QPushButton *refreshButton;
    QListWidget *shortcutList;
    QLineEdit *searchBox;
    QComboBox *categoryFilter;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QLabel *folderPathLabel;
    
    // Menu components
    QMenuBar *menuBar;
    QMenu *ayarlarMenu;
    QMenu *araclarMenu;
    QAction *hakkindaAction;
    QAction *ayarlarAction;
    QAction *yardimAction;
    QAction *backupAction;
    QAction *restoreAction;
    QAction *statisticsAction;
    
    // File management
    FileManager *fileManager;
    QString sourceFolder;
    QString gamesFolder;
    
    // State
    bool isProcessing;
};

#endif // MAINWINDOW_H
