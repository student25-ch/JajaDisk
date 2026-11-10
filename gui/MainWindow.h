/*
 * JajaDisk - 开源磁盘管理与数据恢复工具
 * Copyright (C) 2026 杨子桐
 * 许可证: AGPL-3.0-or-later
 */
#pragma once

#include <QMainWindow>
#include <QLibrary>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPoint>
#include "PartitionMapWidget.h"
#include "JajaDiskCore.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void refreshDisks();
    void onDiskSelected();
    void backupTable();
    void viewSector();
    void showAbout();
    void showContextMenu(const QPoint &pos); // 右键菜单

private:
    QLibrary m_lib;
    QTreeWidget* m_diskTree = nullptr;
    QTableWidget* m_partTable = nullptr;
    QTextEdit* m_log = nullptr;
    PartitionMapWidget* m_map = nullptr;

    // DLL 函数指针
    int (*pGetPartitions)(const wchar_t*, PartitionInfo*, int) = nullptr;
    int (*pReadSector)(const wchar_t*, unsigned long long, unsigned char*, int) = nullptr;
    int (*pBackupTable)(const wchar_t*, const wchar_t*) = nullptr;

    void log(const QString& s);
    QString currentDiskPath() const;
};