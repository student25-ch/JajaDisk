/*
 * JajaDisk - 开源磁盘管理与数据恢复工具
 * Copyright (C) 2026 杨子桐
 * 许可证: AGPL-3.0-or-later
 */
#include "PartitionMapWidget.h"
#include <QPainter>

PartitionMapWidget::PartitionMapWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(90);
    setMouseTracking(true);
}

void PartitionMapWidget::setPartitions(const QVector<PartitionBlock>& parts, quint64 diskSize) {
    m_parts = parts;
    m_diskSize = diskSize;
    update();
}

QColor PartitionMapWidget::colorFor(const PartitionBlock& p) const {
    QString fs = p.fsType.toUpper();
    if (fs == "NTFS")  return QColor(139, 69, 19);
    if (fs == "FAT32") return QColor(70, 130, 180);
    if (fs == "EXT4")  return QColor(60, 179, 113);
    if (fs == "EXFAT") return QColor(218, 165, 32);
    return QColor(120, 120, 120);
}

void PartitionMapWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_diskSize == 0) {
        p.drawText(rect(), Qt::AlignCenter, "无磁盘数据");
        return;
    }

    const int y = 10;
    const int h = 60;
    const int totalW = width();
    qint64 x = 0;
    qint64 prevEnd = 0;

    for (const auto& part : m_parts) {
        qint64 partStart = (qint64)part.startLba * 512;

        if (partStart > prevEnd) {
            qint64 gap = partStart - prevEnd;
            int gw = (int)(gap * totalW / (qint64)m_diskSize);
            if (gw > 1) {
                p.fillRect(QRect((int)x, y, gw, h), QColor(220, 220, 220));
                x += gw;
            }
        }

        int w = (int)((qint64)part.sizeBytes * totalW / (qint64)m_diskSize);
        if (w < 2) w = 2;
        QRect r((int)x, y, w, h);
        p.fillRect(r, colorFor(part));

        p.setPen(Qt::white);
        p.drawText(r, Qt::AlignCenter,
                   QString("%1\n分区%2\n%3 GB")
                       .arg(part.label)
                       .arg(part.partNumber)
                       .arg(part.sizeBytes / 1024.0 / 1024 / 1024, 0, 'f', 1));

        if (part.isActive) {
            p.setPen(QPen(Qt::red, 2));
            p.drawRect(r.adjusted(1, 1, -1, -1));
        }

        x += w;
        prevEnd = partStart + (qint64)part.sizeBytes;
    }

    if (prevEnd < (qint64)m_diskSize) {
        qint64 gap = (qint64)m_diskSize - prevEnd;
        int gw = (int)(gap * totalW / (qint64)m_diskSize);
        if (gw > 1)
            p.fillRect(QRect((int)x, y, gw, h), QColor(220, 220, 220));
    }
}