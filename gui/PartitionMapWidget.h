/*
 * JajaDisk - 开源磁盘管理与数据恢复工具
 * Copyright (C) 2026 杨子桐
 * 许可证: AGPL-3.0-or-later
 */
#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>

struct PartitionBlock {
    quint64 startLba   = 0;
    quint64 sizeBytes  = 0;
    int     partNumber = 0;
    bool    isGpt      = false;
    bool    isActive   = false;
    QString label;
    QString fsType;
};

class PartitionMapWidget : public QWidget {
    Q_OBJECT
public:
    explicit PartitionMapWidget(QWidget* parent = nullptr);
    void setPartitions(const QVector<PartitionBlock>& parts, quint64 diskSize);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QColor colorFor(const PartitionBlock& p) const;
    QVector<PartitionBlock> m_parts;
    quint64 m_diskSize = 0;
};