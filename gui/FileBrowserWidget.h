/* JajaDisk - Copyright (C) 2026 杨子桐 - AGPL-3.0 */
#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLibrary>
#include "JajaDiskCore.h"

class FileBrowserWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileBrowserWidget(QWidget* parent = nullptr);
    void setLibrary(QLibrary* lib);
    void loadDrive(const QString& driveLetter);

private slots:
    void recoverSelectedFile();

private:
    QTableWidget* m_table;
    QLibrary* m_lib;
    QString m_currentDrive;
    int (*pListFiles)(const wchar_t*, FileInfo*, int) = nullptr;
    int (*pRecoverFile)(const wchar_t*, const wchar_t*, const wchar_t*) = nullptr;
};