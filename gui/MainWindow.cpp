/*
 * JajaDisk - 开源磁盘管理与数据恢复工具
 * Copyright (C) 2026 杨子桐
 * 许可证: AGPL-3.0-or-later
 */
#include "MainWindow.h"

#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QApplication>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QDialog>
#include <QFont>
#include <QStorageInfo>  // 用于读取盘符和文件系统
#include <QMenu>         // 右键菜单

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("JajaDisk v0.1 - 磁盘管理工具  |  Copyright (C) 2026 杨子桐  |  AGPL-3.0");
    resize(1100, 700);

    /* ---- 加载核心 DLL ---- */
    m_lib.setFileName(QApplication::applicationDirPath() + "/JajaDiskCore.dll");
    if (m_lib.load()) {
        pGetPartitions = (decltype(pGetPartitions))m_lib.resolve("JajaDisk_GetPartitions");
        pReadSector    = (decltype(pReadSector))m_lib.resolve("JajaDisk_ReadSector");
        pBackupTable   = (decltype(pBackupTable))m_lib.resolve("JajaDisk_BackupPartitionTable");
    }

    /* ---- 菜单栏 ---- */
    auto* helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于 JajaDisk", this, &MainWindow::showAbout);
    helpMenu->addAction("查看 AGPL 许可证", this, []() {
        QDesktopServices::openUrl(QUrl("https://www.gnu.org/licenses/agpl-3.0.html"));
    });

    /* ---- 工具栏 ---- */
    auto* tb = addToolBar("主操作");
    tb->addAction("刷新磁盘", this, &MainWindow::refreshDisks);
    tb->addAction("备份分区表", this, &MainWindow::backupTable);
    tb->addAction("查看扇区", this, &MainWindow::viewSector);

    /* ---- 主布局 ---- */
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    m_diskTree = new QTreeWidget;
    m_diskTree->setHeaderLabel("磁盘 / 分区");
    m_diskTree->setMinimumWidth(280);
    connect(m_diskTree, &QTreeWidget::itemClicked, this, &MainWindow::onDiskSelected);

    // 【开启右键菜单】
    m_diskTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_diskTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);

    auto* right = new QWidget;
    auto* vbox = new QVBoxLayout(right);
    vbox->setContentsMargins(0, 0, 0, 0);

    m_map = new PartitionMapWidget;
    vbox->addWidget(m_map);

    m_partTable = new QTableWidget(0, 5);
    m_partTable->setHorizontalHeaderLabels({"分区号", "起始 LBA", "大小(GB)", "类型", "活动"});
    m_partTable->horizontalHeader()->setStretchLastSection(true);
    vbox->addWidget(m_partTable, 3);

    m_log = new QTextEdit;
    m_log->setReadOnly(true);
    m_log->setMaximumHeight(140);
    vbox->addWidget(m_log);

    splitter->addWidget(m_diskTree);
    splitter->addWidget(right);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    /* ---- 启动日志 ---- */
    log("JajaDisk v0.1  Copyright (C) 2026 杨子桐  AGPL-3.0-or-later");
    if (!m_lib.isLoaded()) {
        log("❌ 核心 DLL 加载失败: " + m_lib.errorString());
    } else {
        log("✅ 核心 DLL 加载成功");
        refreshDisks();
    }
}

MainWindow::~MainWindow() {
    if (m_lib.isLoaded()) m_lib.unload();
}

void MainWindow::log(const QString& s) {
    m_log->append(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + s);
}

void MainWindow::refreshDisks() {
    if (!pGetPartitions) { log("DLL 未就绪"); return; }
    m_diskTree->clear();

    // 获取当前系统挂载的所有盘（用于匹配 C盘、D盘和 NTFS）
    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();

    for (int i = 0; i < 8; i++) {
        QString path = QString("\\\\.\\PhysicalDrive%1").arg(i);
        PartitionInfo parts[128];
        int n = pGetPartitions((const wchar_t*)path.utf16(), parts, 128);
        
        if (n < 0) continue; // 没这个磁盘或者没权限

        auto* diskItem = new QTreeWidgetItem(m_diskTree);
        diskItem->setText(0, QString("物理磁盘 %1 (%2 个分区)").arg(i).arg(n));
        diskItem->setData(0, Qt::UserRole, path); // 存路径

        for (int k = 0; k < n; k++) {
            auto* pi = new QTreeWidgetItem(diskItem);
            double sizeGB = parts[k].sizeBytes / 1024.0 / 1024 / 1024;
            
            QString fsType = "";
            QString driveLetter = "";

            // 遍历系统挂载的卷，匹配大小
            for (const QStorageInfo &storage : volumes) {
                qint64 volSize = storage.bytesTotal();
                qint64 partSize = (qint64)parts[k].sizeBytes;
                
                if (qAbs(volSize - partSize) < 100LL * 1024 * 1024) {
                    fsType = QString::fromUtf8(storage.fileSystemType());
                    driveLetter = storage.rootPath(); 
                    break;
                }
            }

            QString displayText;
            if (!driveLetter.isEmpty()) {
                displayText = QString("%1 [%2] %3 GB")
                              .arg(driveLetter).arg(fsType).arg(QString::number(sizeGB, 'f', 1));
            } else {
                displayText = QString("分区 %1 [未知] %2 GB")
                              .arg(parts[k].partitionNumber).arg(QString::number(sizeGB, 'f', 1));
            }
            pi->setText(0, displayText);
        }
    }
    m_diskTree->expandAll();
    log("磁盘列表已刷新（已尝试匹配盘符和文件系统）");

    // 默认选中第一个磁盘
    if (m_diskTree->topLevelItemCount() > 0) {
        m_diskTree->setCurrentItem(m_diskTree->topLevelItem(0));
        onDiskSelected();
    }
}

QString MainWindow::currentDiskPath() const {
    auto* item = m_diskTree->currentItem();
    if (!item) return {};
    while (item->parent()) item = item->parent();
    return item->data(0, Qt::UserRole).toString();
}

void MainWindow::onDiskSelected() {
    QString path = currentDiskPath();
    if (path.isEmpty() || !pGetPartitions) return;

    PartitionInfo parts[128];
    int n = pGetPartitions((const wchar_t*)path.utf16(), parts, 128);
    if (n < 0) return;

    m_partTable->setRowCount(n);
    QVector<PartitionBlock> blocks;
    quint64 maxEnd = 0;

    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();

    for (int i = 0; i < n; i++) {
        auto& p = parts[i];
        // 填充下方表格
        m_partTable->setItem(i, 0, new QTableWidgetItem(QString::number(p.partitionNumber)));
        m_partTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.startLba)));
        m_partTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(p.sizeBytes / 1024.0 / 1024 / 1024, 'f', 2)));
        m_partTable->setItem(i, 3, new QTableWidgetItem(p.isGpt ? "GPT" : "MBR"));
        m_partTable->setItem(i, 4, new QTableWidgetItem(p.isActive ? "是" : "否"));

        // 填充上方色块图的数据
        PartitionBlock b;
        b.startLba   = p.startLba;
        b.sizeBytes  = p.sizeBytes;
        b.partNumber = p.partitionNumber;
        b.isGpt      = p.isGpt;
        b.isActive   = p.isActive;
        
        // 匹配文件系统，用来给色块图上色
        QString fsType = "未分配";
        for (const QStorageInfo &storage : volumes) {
            if (qAbs(storage.bytesTotal() - (qint64)p.sizeBytes) < 100LL * 1024 * 1024) {
                fsType = QString::fromUtf8(storage.fileSystemType());
                b.label = storage.rootPath(); // 显示盘符
                break;
            }
        }
        
        b.fsType = fsType;
        if (b.label.isEmpty()) b.label = QString("分区%1").arg(p.partitionNumber);
        
        blocks.append(b);

        quint64 end = p.startLba * 512 + p.sizeBytes;
        if (end > maxEnd) maxEnd = end;
    }

    // 防止画图失败
    if (maxEnd == 0) maxEnd = 500ULL * 1024 * 1024 * 1024; 

    m_map->setPartitions(blocks, maxEnd);
    log(QString("已加载磁盘结构，识别到 %1 个分区").arg(n));
}

void MainWindow::showContextMenu(const QPoint &pos) {
    QTreeWidgetItem* item = m_diskTree->currentItem();
    if (!item) return;

    QMenu contextMenu(this);
    QAction* actRefresh = contextMenu.addAction("刷新磁盘");
    QAction* actBackup = contextMenu.addAction("备份分区表");
    contextMenu.addSeparator();
    QAction* actViewSector = contextMenu.addAction("查看扇区 (Hex)");
    contextMenu.addSeparator();
    QAction* actAbout = contextMenu.addAction("关于 JajaDisk");

    QAction* selectedAction = contextMenu.exec(m_diskTree->mapToGlobal(pos));

    if (selectedAction == actRefresh) {
        refreshDisks();
    } else if (selectedAction == actBackup) {
        backupTable();
    } else if (selectedAction == actViewSector) {
        viewSector();
    } else if (selectedAction == actAbout) {
        showAbout();
    }
}

void MainWindow::backupTable() {
    QString disk = currentDiskPath();
    if (disk.isEmpty()) { log("请先选择磁盘"); return; }
    if (!pBackupTable) { log("DLL 未就绪"); return; }

    QString dir = QApplication::applicationDirPath() + "/backup";
    QDir().mkpath(dir);
    QString file = QFileDialog::getSaveFileName(this, "保存分区表备份",
        dir + "/parttable_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".bin",
        "分区表备份 (*.bin)");
    if (file.isEmpty()) return;

    int r = pBackupTable((const wchar_t*)disk.utf16(), (const wchar_t*)file.utf16());
    if (r == 0) {
        log("✅ 分区表已备份: " + file);
        QMessageBox::information(this, "成功", "分区表已备份到:\n" + file);
    } else {
        log(QString("❌ 备份失败, 错误码 %1").arg(r));
    }
}

void MainWindow::viewSector() {
    QString disk = currentDiskPath();
    if (disk.isEmpty() || !pReadSector) { log("请选择磁盘"); return; }

    bool ok = false;
    int lba = QInputDialog::getInt(this, "查看扇区", "输入 LBA:", 0, 0, 2147483647, 1, &ok);
    if (!ok) return;

    unsigned char buf[512];
    int n = pReadSector((const wchar_t*)disk.utf16(), lba, buf, 512);
    if (n <= 0) { log("读扇区失败"); return; }

    QString hex;
    for (int i = 0; i < n; i++) {
        hex += QString("%1 ").arg(buf[i], 2, 16, QChar('0')).toUpper();
        if ((i + 1) % 16 == 0) hex += "\n";
    }

    QDialog dlg(this);
    dlg.setWindowTitle(QString("LBA %1").arg(lba));
    dlg.resize(640, 480);
    auto* te = new QTextEdit(&dlg);
    te->setFont(QFont("Consolas", 10));
    te->setPlainText(hex);
    auto* lay = new QVBoxLayout(&dlg);
    lay->addWidget(te);
    dlg.exec();
}

void MainWindow::showAbout() {
    QMessageBox::about(this, "关于 JajaDisk",
        "<h3>JajaDisk v0.1</h3>"
        "<p>开源磁盘管理与数据恢复工具</p>"
        "<p><b>Copyright (C) 2026 杨子桐</b></p>"
        "<p>本程序是自由软件，您可以根据自由软件基金会发布的 "
        "GNU Affero 通用公共许可证（AGPL）第 3 版或任何更新版本的条款"
        "重新分发和/或修改它。</p>"
        "<p>许可证: AGPL-3.0-or-later<br>"
        "详情: <a href='https://www.gnu.org/licenses/'>https://www.gnu.org/licenses/</a></p>");
}