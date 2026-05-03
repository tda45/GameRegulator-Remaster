#include "mainwindow.h"
#include <QApplication>
#include <QStyle>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QTranslator>
#include <QLocale>
#include <QTextStream>
#include <QProcess>
#include <QDirIterator>
#include <QDateTime>
#include <QMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , mainLayout(nullptr)
    , buttonLayout(nullptr)
    , searchLayout(nullptr)
    , tabWidget(nullptr)
    , selectFolderButton(nullptr)
    , organizeButton(nullptr)
    , backupButton(nullptr)
    , restoreButton(nullptr)
    , statisticsButton(nullptr)
    , refreshButton(nullptr)
    , shortcutList(nullptr)
    , searchBox(nullptr)
    , categoryFilter(nullptr)
    , statusLabel(nullptr)
    , progressBar(nullptr)
    , folderPathLabel(nullptr)
    , menuBar(nullptr)
    , ayarlarMenu(nullptr)
    , araclarMenu(nullptr)
    , hakkindaAction(nullptr)
    , ayarlarAction(nullptr)
    , backupAction(nullptr)
    , restoreAction(nullptr)
    , statisticsAction(nullptr)
    , fileManager(nullptr)
    , isProcessing(false)
{
    fileManager = new FileManager(this);
    connect(fileManager, &FileManager::progressUpdated, this, &MainWindow::updateProgress);
    connect(fileManager, &FileManager::operationCompleted, this, [this](bool success, const QString &message) {
        isProcessing = false;
        organizeButton->setEnabled(false);
        if (success) {
            QMessageBox::information(this, tr("Başarılı"), message);
            resetUI();
        } else {
            QMessageBox::critical(this, tr("Hata"), message);
        }
    });
    connect(fileManager, &FileManager::errorOccurred, this, [this](const QString &error) {
        isProcessing = false;
        organizeButton->setEnabled(false);
        QMessageBox::critical(this, tr("Hata"), error);
    });

    setupUI();
    connectSignals();
    setWindowTitle(tr("GameRegulator Remaster"));
    setMinimumSize(600, 400);
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create menu bar
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // Create menus
    ayarlarMenu = new QMenu(tr("Ayarlar"), this);
    araclarMenu = new QMenu("Araçlar", this);
    menuBar->addMenu(ayarlarMenu);
    menuBar->addMenu(araclarMenu);
    
    // Add menu actions
    hakkindaAction = new QAction(tr("Hakkında"), this);
    ayarlarAction = new QAction(tr("Ayarlar"), this);
    yardimAction = new QAction("Yardım", this);
    backupAction = new QAction("Yedekle", this);
    restoreAction = new QAction("Geri Yükle", this);
    statisticsAction = new QAction("İstatistikler", this);
    
    ayarlarMenu->addAction(hakkindaAction);
    ayarlarMenu->addAction(ayarlarAction);
    ayarlarMenu->addAction(yardimAction);
    araclarMenu->addAction(backupAction);
    araclarMenu->addAction(restoreAction);
    araclarMenu->addAction(statisticsAction);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Top section - folder selection
    folderPathLabel = new QLabel(tr("Kaynak klasör seçilmedi"), this);
    folderPathLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    mainLayout->addWidget(folderPathLabel);

    selectFolderButton = new QPushButton(tr("Klasör Seç"), this);
    selectFolderButton->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    selectFolderButton->setMinimumHeight(40);
    mainLayout->addWidget(selectFolderButton);

    // Search and filter section
    searchLayout = new QHBoxLayout();
    
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Kısayol ara...");
    searchBox->setClearButtonEnabled(true);
    searchLayout->addWidget(searchBox);
    
    categoryFilter = new QComboBox(this);
    categoryFilter->addItem("Tüm Kategoriler", "all");
    categoryFilter->addItem("Action", "action");
    categoryFilter->addItem("RPG", "rpg");
    categoryFilter->addItem("Strategy", "strategy");
    categoryFilter->addItem("Simulation", "simulation");
    categoryFilter->addItem("Sports", "sports");
    categoryFilter->addItem("Racing", "racing");
    categoryFilter->addItem("Diğer", "other");
    searchLayout->addWidget(categoryFilter);
    
    refreshButton = new QPushButton("Yenile", this);
    refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    searchLayout->addWidget(refreshButton);
    
    mainLayout->addLayout(searchLayout);

    // Middle section - shortcuts list
    QLabel *listLabel = new QLabel(tr("Bulunan Oyun Kısayolları:"), this);
    listLabel->setStyleSheet("QLabel { font-weight: bold; margin-top: 10px; }");
    mainLayout->addWidget(listLabel);

    shortcutList = new QListWidget(this);
    shortcutList->setSelectionMode(QAbstractItemView::MultiSelection);
    shortcutList->setMinimumHeight(300);
    shortcutList->setDragEnabled(true);
    shortcutList->setAcceptDrops(true);
    shortcutList->setDropIndicatorShown(true);
    mainLayout->addWidget(shortcutList);

    // Bottom section - buttons and status
    buttonLayout = new QHBoxLayout();
    
    organizeButton = new QPushButton(tr("Kısayolları Klasöre Ekle"), this);
    organizeButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    organizeButton->setMinimumHeight(40);
    organizeButton->setEnabled(false);
    
    backupButton = new QPushButton("Klasörü Yedekle", this);
    backupButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    backupButton->setMinimumHeight(40);
    
    restoreButton = new QPushButton("Klasörü Geri Yükle", this);
    restoreButton->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    restoreButton->setMinimumHeight(40);
    
    statisticsButton = new QPushButton("İstatistikler", this);
    statisticsButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogInfoView));
    statisticsButton->setMinimumHeight(40);
    
    buttonLayout->addWidget(organizeButton);
    buttonLayout->addWidget(backupButton);
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(statisticsButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    // Status label
    statusLabel = new QLabel(tr("Hazır"), this);
    statusLabel->setStyleSheet("QLabel { color: #666; }");
    mainLayout->addWidget(statusLabel);
}

void MainWindow::connectSignals()
{
    connect(selectFolderButton, &QPushButton::clicked, this, &MainWindow::selectSourceFolder);
    connect(organizeButton, &QPushButton::clicked, this, &MainWindow::organizeShortcuts);
    connect(backupButton, &QPushButton::clicked, this, &MainWindow::backupShortcuts);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreShortcuts);
    connect(statisticsButton, &QPushButton::clicked, this, &MainWindow::showStatistics);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshShortcutList);
    connect(shortcutList, &QListWidget::itemSelectionChanged, this, &MainWindow::onShortcutSelectionChanged);
    connect(searchBox, &QLineEdit::textChanged, this, &MainWindow::filterShortcuts);
    connect(categoryFilter, &QComboBox::currentTextChanged, this, &MainWindow::filterByCategory);
    connect(hakkindaAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ayarlarAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(yardimAction, &QAction::triggered, this, &MainWindow::showHelpDialog);
    connect(backupAction, &QAction::triggered, this, &MainWindow::backupShortcuts);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::restoreShortcuts);
    connect(statisticsAction, &QAction::triggered, this, &MainWindow::showStatistics);
}

void MainWindow::selectSourceFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Oyun Kısayollarının Bulunduğu Klasörü Seçin"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folderPath.isEmpty()) {
        sourceFolder = folderPath;
        folderPathLabel->setText(QString(tr("Seçilen Klasör: %1")).arg(sourceFolder));
        folderPathLabel->setStyleSheet("QLabel { color: #333; font-style: normal; }");
        
        scanForShortcuts(sourceFolder);
    }
}

void MainWindow::scanForShortcuts(const QString &folderPath)
{
    shortcutList->clear();
    statusLabel->setText(tr("Kısayollar taranıyor..."));
    
    QList<ShortcutInfo> shortcuts = fileManager->findShortcuts(folderPath);
    
    if (shortcuts.isEmpty()) {
        statusLabel->setText(tr("Bu klasörde oyun kısayolu bulunamadı."));
        organizeButton->setEnabled(false);
        return;
    }

    for (const auto &shortcut : shortcuts) {
        // Check if file actually exists before adding to list
        if (!QFile::exists(shortcut.filePath)) {
            qDebug() << "Skipping non-existent file:" << shortcut.filePath;
            continue;
        }
        
        QString displayText = shortcut.fileName;
        if (!shortcut.creationTime.isEmpty()) {
            displayText += QString(" (%1)").arg(shortcut.creationTime);
        }
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, shortcut.filePath);
        item->setData(Qt::UserRole + 1, shortcut.creationTime);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        shortcutList->addItem(item);
    }

    statusLabel->setText(QString(tr("%1 oyun kısayolu bulundu")).arg(shortcuts.size()));
    organizeButton->setEnabled(true);
}

void MainWindow::onShortcutSelectionChanged()
{
    int selectedCount = 0;
    for (int i = 0; i < shortcutList->count(); ++i) {
        QListWidgetItem *item = shortcutList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedCount++;
        }
    }
    
    organizeButton->setEnabled(selectedCount > 0 && !isProcessing);
    if (selectedCount > 0) {
        statusLabel->setText(QString(tr("%1 kısayol seçildi")).arg(selectedCount));
    } else {
        statusLabel->setText(tr("Lütfen düzenlenecek kısayolları seçin"));
    }
}

void MainWindow::organizeShortcuts()
{
    if (sourceFolder.isEmpty()) {
        QMessageBox::warning(this, tr("Uyarı"), tr("Lütfen önce bir kaynak klasör seçin."));
        return;
    }

    QList<ShortcutInfo> selectedShortcuts;
    
    for (int i = 0; i < shortcutList->count(); ++i) {
        QListWidgetItem *item = shortcutList->item(i);
        if (item->checkState() == Qt::Checked) {
            QString filePath = item->data(Qt::UserRole).toString();
            
            // Double-check file exists before processing
            if (!QFile::exists(filePath)) {
                qDebug() << "File no longer exists, skipping:" << filePath;
                continue;
            }
            
            ShortcutInfo shortcut;
            shortcut.filePath = filePath;
            shortcut.fileName = item->text().split(" (").first();
            shortcut.isSelected = true;
            selectedShortcuts.append(shortcut);
        }
    }

    if (selectedShortcuts.isEmpty()) {
        QMessageBox::warning(this, tr("Uyarı"), tr("Lütfen düzenlenecek en az bir kısayol seçin."));
        return;
    }

    isProcessing = true;
    organizeButton->setEnabled(false);
    progressBar->setVisible(true);
    progressBar->setValue(0);
    
    statusLabel->setText(tr("Kısayollar düzenleniyor..."));
    
    // Debug: Log how many shortcuts we're trying to move
    qDebug() << "DEBUG: organizeShortcuts called with" << selectedShortcuts.size() << "shortcuts";
    for (int i = 0; i < selectedShortcuts.size(); i++) {
        qDebug() << "DEBUG: Shortcut" << i << ":" << selectedShortcuts[i].fileName << selectedShortcuts[i].filePath;
    }
    
    // Move shortcuts (this will create the folder internally)
    fileManager->moveSelectedShortcuts(selectedShortcuts);
}

void MainWindow::updateProgress(int value, const QString &message)
{
    progressBar->setValue(value);
    statusLabel->setText(message);
}

void MainWindow::createGamesFolder()
{
    fileManager->createGamesFolder();
}

void MainWindow::resetUI()
{
    shortcutList->clear();
    folderPathLabel->setText(tr("Kaynak klasör seçilmedi"));
    folderPathLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    sourceFolder.clear();
    progressBar->setVisible(false);
    progressBar->setValue(0);
    organizeButton->setEnabled(false);
    statusLabel->setText(tr("Hazır"));
}

void MainWindow::filterShortcuts(const QString &text)
{
    for (int i = 0; i < shortcutList->count(); ++i) {
        QListWidgetItem *item = shortcutList->item(i);
        QString itemText = item->text().toLower();
        bool visible = itemText.contains(text.toLower());
        item->setHidden(!visible);
    }
}

void MainWindow::filterByCategory(const QString &category)
{
    // This will be implemented with category system later
    filterShortcuts(searchBox->text());
}

void MainWindow::refreshShortcutList()
{
    if (!sourceFolder.isEmpty()) {
        scanForShortcuts(sourceFolder);
    }
}

void MainWindow::backupShortcuts()
{
    QString backupPath = QFileDialog::getExistingDirectory(
        this,
        "Yedekleme Klasörünü Seçin",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (backupPath.isEmpty()) return;
    
    QString userProfile = qgetenv("USERPROFILE");
    QString gamesFolder = userProfile + "/Desktop/Oyunlar";
    
    QDir gamesDir(gamesFolder);
    if (!gamesDir.exists()) {
        QMessageBox::warning(this, "Uyarı", "Oyunlar klasörü bulunamadı!");
        return;
    }
    
    QString backupFolder = backupPath + "/GameRegulator_Backup_" + QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss");
    QDir().mkpath(backupFolder);
    
    // Copy all shortcuts
    int copiedCount = 0;
    QStringList shortcuts = gamesDir.entryList(QStringList() << "*.lnk", QDir::Files);
    
    for (const QString &shortcut : shortcuts) {
        QString source = gamesFolder + "/" + shortcut;
        QString dest = backupFolder + "/" + shortcut;
        
        if (QFile::copy(source, dest)) {
            copiedCount++;
        }
    }
    
    QMessageBox::information(this, "Yedekleme Tamamlandı", 
        QString("%1 kısayol başarıyla yedeklendi.\nKlasör: %2").arg(copiedCount).arg(backupFolder));
}

void MainWindow::restoreShortcuts()
{
    QString restorePath = QFileDialog::getExistingDirectory(
        this,
        "Yedekleme Klasörünü Seçin",
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (restorePath.isEmpty()) return;
    
    QString userProfile = qgetenv("USERPROFILE");
    QString gamesFolder = userProfile + "/Desktop/Oyunlar";
    
    QDir().mkpath(gamesFolder);
    
    int restoredCount = 0;
    QStringList shortcuts = QDir(restorePath).entryList(QStringList() << "*.lnk", QDir::Files);
    
    for (const QString &shortcut : shortcuts) {
        QString source = restorePath + "/" + shortcut;
        QString dest = gamesFolder + "/" + shortcut;
        
        if (QFile::copy(source, dest)) {
            restoredCount++;
        }
    }
    
    QMessageBox::information(this, "Geri Yükleme Tamamlandı", 
        QString("%1 kısayol başarıyla geri yüklendi.").arg(restoredCount));
    
    // Refresh the list
    if (!sourceFolder.isEmpty()) {
        scanForShortcuts(sourceFolder);
    }
}

void MainWindow::showStatistics()
{
    QDialog statsDialog(this);
    statsDialog.setWindowTitle("İstatistikler - GameRegulator Remaster");
    statsDialog.setFixedSize(500, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&statsDialog);
    
    QTextEdit *statsText = new QTextEdit(&statsDialog);
    statsText->setReadOnly(true);
    
        
    // Count shortcuts in Games folder
    QString userProfile = qgetenv("USERPROFILE");
    QString gamesFolder = userProfile + "/Desktop/Oyunlar";
    
    QDir gamesDir(gamesFolder);
    int totalShortcuts = gamesDir.entryList(QStringList() << "*.lnk", QDir::Files).count();
    
    // Count selected shortcuts
    int selectedShortcuts = 0;
    for (int i = 0; i < shortcutList->count(); i++) {
        if (shortcutList->item(i)->isSelected()) {
            selectedShortcuts++;
        }
    }
    
    // Display statistics
    QString statsInfo = QString(
        "Toplam Kısayol: %1\n"
        "Seçili Kısayol: %2\n"
        "Seçim Oranı: %3%"
    ).arg(totalShortcuts)
     .arg(selectedShortcuts)
     .arg(totalShortcuts > 0 ? (selectedShortcuts * 100 / totalShortcuts) : 0);
    
    statsText->setPlainText(statsInfo);
    layout->addWidget(statsText);
    
    // Close button
    QPushButton *closeButton = new QPushButton(tr("Kapat"), &statsDialog);
    layout->addWidget(closeButton);
    
    connect(closeButton, &QPushButton::clicked, &statsDialog, &QDialog::accept);
    
    statsDialog.exec();
}

void MainWindow::showHelpDialog()
{
    // Create help dialog
    QDialog helpDialog(this);
    helpDialog.setWindowTitle(tr("GameRegulator Remaster - Yardım"));
    helpDialog.setMinimumSize(600, 500);
    helpDialog.resize(700, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(&helpDialog);
    
    // Help content
    QString helpText = QString(
        "<h2>GameRegulator Remaster Kullanım Kılavuzu</h2>"
        "<h3>🎮 Temel Kullanım</h3>"
        "<p><b>1. Klasör Seçimi:</b> Oyun kısayollarınızın bulunduğu klasörü seçin.</p>"
        "<p><b>2. Kısayol Seçimi:</b> Taşımak istediğiniz kısayolları seçin.</p>"
        "<p><b>3. Organize Et:</b> 'Kısayolları Düzenle' butonuna tıklayarak kısayolları Oyunlar klasörüne taşıyın.</p>"
        
        "<h3>🔧 Özellikler</h3>"
        "<p><b>• Kısayol Doğrulama:</b> Kırık veya geçersiz kısayolları otomatik tespit eder.</p>"
        "<p><b>• İstatistik Paneli:</b> Kısayol sayısı ve kullanım istatistiklerini gösterir.</p>"
        "<p><b>• Yedekleme/Geri Yükleme:</b> Kısayollarınızı yedekleyip geri yükleyebilirsiniz.</p>"
        
        "<h3>⚙️ Ayarlar</h3>"
        "<p><b>• Klasör Adı:</b> Oyunlar klasörünün adını değiştirebilirsiniz.</p>"
        "<p><b>• Tema Seçimi:</b> Uygulama görünümünü değiştirebilirsiniz.</p>"
        
        "<h3>📁 Klasör Yapısı</h3>"
        "<p><b>Source Folder:</b> Oyun kısayollarınızın bulunduğu klasör</p>"
        "<p><b>Games Folder:</b> Oyunlar klasörü (OneDrive/Desktop/Oyunlar)</p>"
        
        "<h3>🔍 İpuçları</h3>"
        "<p><b>• Arama:</b> Kısayollar arasında hızlı arama yapın.</p>"
        "<p><b>• Filtreleme:</b> Kısayolları filtreleyin.</p>"
        "<p><b>• Toplu Seçim:</b> Ctrl+A ile tüm kısayolları seçin.</p>"
        
        "<h3>⚠️ Sorun Giderme</h3>"
        "<p><b>• Klasör Görünmüyor:</b> OneDrive Desktop'ta klasör görünmüyorsa, birkaç saniye bekleyin.</p>"
        "<p><b>• Kısayol Çalışmıyor:</b> Kısayol hedefi taşınmışsa, kısayolu silip yeniden oluşturun.</p>"
        
        "<h3>💡 Notlar</h3>"
        "<p><b>• OneDrive Desktop:</b> Klasör otomatik olarak OneDrive Desktop'ta oluşturulur.</p>"
        "<p><b>• Otomatik Yedekleme:</b> Kısayollar düzenlenirken otomatik olarak yedeklenir.</p>"
    );
    
    QTextEdit *helpTextEdit = new QTextEdit(&helpDialog);
    helpTextEdit->setHtml(helpText);
    helpTextEdit->setReadOnly(true);
    helpTextEdit->setStyleSheet("QTextEdit { font-size: 12px; padding: 10px; }");
    layout->addWidget(helpTextEdit);
    
    // Close button
    QPushButton *closeButton = new QPushButton(tr("Kapat"), &helpDialog);
    layout->addWidget(closeButton);
    
    connect(closeButton, &QPushButton::clicked, &helpDialog, &QDialog::accept);
    
    helpDialog.exec();
}

void MainWindow::showAboutDialog()
{
    QDialog aboutDialog(this);
    aboutDialog.setWindowTitle(tr("Hakkında - GameRegulator Remaster"));
    aboutDialog.setFixedSize(400, 300);
    
    QVBoxLayout *layout = new QVBoxLayout(&aboutDialog);
    
    QLabel *titleLabel = new QLabel("GameRegulator Remaster", &aboutDialog);
    titleLabel->setStyleSheet("QLabel { font-size: 18px; font-weight: bold; color: #2c3e50; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *versionLabel = new QLabel(tr("Sürüm: 1.0.0"), &aboutDialog);
    versionLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *authorLabel = new QLabel(tr("Yapımcı: tda_45"), &aboutDialog);
    authorLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *linkLabel = new QLabel(tr("Proje Linki:"), &aboutDialog);
    linkLabel->setAlignment(Qt::AlignCenter);
    
    QLabel *urlLabel = new QLabel("<a href='https://github.com/tda45/GameRegulator-Remaster'>https://github.com/tda45/GameRegulator-Remaster</a>", &aboutDialog);
    urlLabel->setAlignment(Qt::AlignCenter);
    urlLabel->setOpenExternalLinks(true);
    
    QLabel *descriptionLabel = new QLabel(tr("Windows için oyun kısayolu düzenleme aracı"), &aboutDialog);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);
    
    QPushButton *closeButton = new QPushButton(tr("Kapat"), &aboutDialog);
    
    layout->addWidget(titleLabel);
    layout->addWidget(versionLabel);
    layout->addWidget(authorLabel);
    layout->addWidget(linkLabel);
    layout->addWidget(urlLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();
    layout->addWidget(closeButton);
    
    connect(closeButton, &QPushButton::clicked, &aboutDialog, &QDialog::accept);
    
    aboutDialog.exec();
}

void MainWindow::showSettingsDialog()
{
    QDialog settingsDialog(this);
    settingsDialog.setWindowTitle("Ayarlar - GameRegulator Remaster");
    settingsDialog.setFixedSize(500, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(&settingsDialog);
    
    // Tema seçimi
    QGroupBox *themeGroup = new QGroupBox("Tema Seçimi", &settingsDialog);
    QFormLayout *themeLayout = new QFormLayout(themeGroup);
    
    QComboBox *themeCombo = new QComboBox(&settingsDialog);
    themeCombo->addItem("Varsayılan", "default");
    themeCombo->addItem("Koyu", "dark");
    themeCombo->addItem("Açık", "light");
    
    // Klasör ayarları - sadece isim değiştirme
    QGroupBox *folderGroup = new QGroupBox("Klasör Ayarları", &settingsDialog);
    QFormLayout *folderLayout = new QFormLayout(folderGroup);
    
    QLineEdit *folderNameEdit = new QLineEdit(&settingsDialog);
    folderNameEdit->setPlaceholderText("Oyunlar");
    
    folderLayout->addRow("Klasör Adı:", folderNameEdit);
    
    // Mevcut ayarları settings.ini'den oku
    QString userProfile = qgetenv("USERPROFILE");
    QString customAppData = userProfile + "/AppData/Local/GameRegulatorRemasterData";
    QString settingsPath = customAppData + "/settings.ini";
    QString currentTheme = "default";
    QString currentFolderName = "Oyunlar";
    
    QFile settingsFile(settingsPath);
    if (settingsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&settingsFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains("=")) {
                QString key = line.split("=")[0].trimmed();
                QString value = line.split("=")[1].trimmed();
                if (key == "theme") {
                    currentTheme = value;
                } else if (key == "folder_name") {
                    currentFolderName = value;
                }
            }
        }
        settingsFile.close();
    }
    
    // Mevcut ayarları UI'a yükle
    themeCombo->setCurrentIndex(themeCombo->findData(currentTheme));
    folderNameEdit->setText(currentFolderName);
    
    themeLayout->addRow("Tema:", themeCombo);
    
    QPushButton *saveButton = new QPushButton("Kaydet", &settingsDialog);
    QPushButton *cancelButton = new QPushButton("İptal", &settingsDialog);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addWidget(themeGroup);
    layout->addWidget(folderGroup);
    layout->addStretch();
    layout->addLayout(buttonLayout);
    
        
    connect(saveButton, &QPushButton::clicked, &settingsDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &settingsDialog, &QDialog::reject);
    
    if (settingsDialog.exec() == QDialog::Accepted) {
        QString selectedTheme = themeCombo->currentData().toString();
        QString selectedFolderName = folderNameEdit->text().trimmed();
        
        // Ayarları kaydet - sadece tema ve klasör adı
        QDir().mkpath(customAppData);
        
        // Önce mevcut ayarları oku
        QMap<QString, QString> allSettings;
        QFile existingFile(settingsPath);
        if (existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&existingFile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains("=")) {
                    QString key = line.split("=")[0].trimmed();
                    QString value = line.split("=")[1].trimmed();
                    allSettings[key] = value;
                }
            }
            existingFile.close();
        }
        
        // Yeni ayarları güncelle
        allSettings["theme"] = selectedTheme;
        allSettings["folder_name"] = selectedFolderName;
        
        // Tüm ayarları kaydet
        QFile settingsFile(settingsPath);
        if (settingsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&settingsFile);
            for (auto it = allSettings.begin(); it != allSettings.end(); ++it) {
                out << it.key() << "=" << it.value() << "\n";
            }
            settingsFile.close();
        }
        
        // Klasör adı değişikliği kontrolü
        bool folderNameChanged = (selectedFolderName != currentFolderName);
        bool themeChanged = (selectedTheme != currentTheme);
        
        if (folderNameChanged || themeChanged) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, 
                "Ayar Değişikliği",
                "Ayar değişikliği için uygulama yeniden başlatılmalıdır. Devam etmek istiyor musunuz?",
                QMessageBox::Yes | QMessageBox::No
            );
            
            if (reply == QMessageBox::Yes) {
                // Uygulamayı yeniden başlat
                QCoreApplication::exit(0);
                QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
                return;
            }
        }
        
        QMessageBox::information(this, "Ayarlar", 
            QString("Tema: %1\nKlasör Adı: %2\n\nAyarlar kaydedildi")
            .arg(selectedTheme == "default" ? "Varsayılan" : 
                 selectedTheme == "dark" ? "Koyu" : "Açık")
            .arg(selectedFolderName));
    }
}
