/* JajaDisk - Copyright (C) 2026 杨子桐 - AGPL-3.0 */
#include "FileBrowserWidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

FileBrowserWidget::FileBrowserWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"文件名", "大小", "类型"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_table);

    auto* btnRecover = new QPushButton("恢复选中的文件");
    layout->addWidget(btnRecover);
    connect(btnRecover, &QPushButton::clicked, this, &FileBrowserWidget::recoverSelectedFile);
}

void FileBrowserWidget::setLibrary(QLibrary* lib) {
    m_lib = lib;
    if (m_lib && m_lib->isLoaded()) {
        pListFiles = (decltype(pListFiles))m_lib->resolve("JajaDisk_ListFiles");
        pRecoverFile = (decltype(pRecoverFile))m_lib->resolve("JajaDisk_RecoverFile");
    }
}

void FileBrowserWidget::loadDrive(const QString& driveLetter) {
    m_currentDrive = driveLetter;
    if (!pListFiles || driveLetter.isEmpty()) return;

    m_table->setRowCount(0);
    FileInfo files[1024];
    int n = pListFiles((const wchar_t*)driveLetter.utf16(), files, 1024);
    if (n < 0) return;

    m_table->setRowCount(n);
    for (int i = 0; i < n; i++) {
        m_table->setItem(i, 0, new QTableWidgetItem(QString::fromWCharArray(files[i].name)));
        m_table->setItem(i, 1, new QTableWidgetItem(QString::number(files[i].sizeBytes) + " 字节"));
        m_table->setItem(i, 2, new QTableWidgetItem(files[i].isDirectory ? "文件夹" : "文件"));
    }
}

void FileBrowserWidget::recoverSelectedFile() {
    int row = m_table->currentRow();
    if (row < 0) return;
    
    QString fileName = m_table->item(row, 0)->text();
    QString save = QFileDialog::getSaveFileName(this, "保存恢复文件", fileName);
    if (save.isEmpty()) return;

    if (pRecoverFile) {
        int r = pRecoverFile((const wchar_t*)m_currentDrive.utf16(), 
                             (const wchar_t*)fileName.utf16(), 
                             (const wchar_t*)save.utf16());
        if (r == 0) QMessageBox::information(this, "成功", "文件已成功恢复！");
        else QMessageBox::warning(this, "失败", "文件恢复失败！");
    }
}