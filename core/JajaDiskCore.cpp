/*
 * JajaDisk - 开源磁盘管理与数据恢复工具
 * Copyright (C) 2026 杨子桐
 * 许可证: AGPL-3.0-or-later
 */
#define JAJADISKCORE_EXPORTS
#include "JajaDiskCore.h"

#include <windows.h>
#include <winioctl.h>
#include <cstdio>
#include <cwchar>
#include <cstring>
#include <cstdlib>

static HANDLE OpenDisk(const wchar_t* path) {
    return CreateFileW(path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);
}

extern "C" JAJA_API int JajaDisk_GetPartitions(const wchar_t* diskPath,
                                               PartitionInfo* outBuf,
                                               int maxCount) {
    if (!diskPath || !outBuf || maxCount <= 0) return -1;

    HANDLE h = OpenDisk(diskPath);
    if (h == INVALID_HANDLE_VALUE) return -2;

    DWORD bufSize = sizeof(DRIVE_LAYOUT_INFORMATION_EX)
                  + 128 * sizeof(PARTITION_INFORMATION_EX);
    auto* layout = (DRIVE_LAYOUT_INFORMATION_EX*)malloc(bufSize);
    if (!layout) { CloseHandle(h); return -4; }

    DWORD ret = 0;
    BOOL ok = DeviceIoControl(h, IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                              NULL, 0, layout, bufSize, &ret, NULL);
    if (!ok) { free(layout); CloseHandle(h); return -3; }

    int count = 0;
    for (DWORD i = 0; i < layout->PartitionCount && count < maxCount; i++) {
        auto& p = layout->PartitionEntry[i];
        if (p.PartitionLength.QuadPart == 0) continue;

        PartitionInfo& info = outBuf[count];
        memset(&info, 0, sizeof(info));
        info.startLba        = p.StartingOffset.QuadPart / 512;
        info.sizeBytes       = p.PartitionLength.QuadPart;
        info.partitionNumber = p.PartitionNumber;
        info.isGpt           = (layout->PartitionStyle == PARTITION_STYLE_GPT);

        if (p.PartitionStyle == PARTITION_STYLE_MBR)
            info.isActive = p.Mbr.BootIndicator;

        wcscpy_s(info.label, L"(未挂载)");
        wcscpy_s(info.fsType, L"");
        count++;
    }
    free(layout);
    CloseHandle(h);
    return count;
}

extern "C" JAJA_API int JajaDisk_ReadSector(const wchar_t* diskPath,
                                            unsigned long long lba,
                                            unsigned char* buf,
                                            int bufSize) {
    if (!diskPath || !buf || bufSize <= 0) return -1;

    HANDLE h = OpenDisk(diskPath);
    if (h == INVALID_HANDLE_VALUE) return -2;

    LARGE_INTEGER off;
    off.QuadPart = (LONGLONG)lba * 512;
    SetFilePointerEx(h, off, NULL, FILE_BEGIN);

    DWORD read = 0;
    BOOL ok = ReadFile(h, buf, (DWORD)bufSize, &read, NULL);
    CloseHandle(h);
    return ok ? (int)read : -3;
}

extern "C" JAJA_API int JajaDisk_BackupPartitionTable(const wchar_t* diskPath,
                                                       const wchar_t* outFile) {
    if (!diskPath || !outFile) return -1;

    unsigned char buf[512 * 34];
    int n = JajaDisk_ReadSector(diskPath, 0, buf, sizeof(buf));
    if (n <= 0) return -2;

    HANDLE f = CreateFileW(outFile, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) return -3;

    DWORD written = 0;
    WriteFile(f, buf, (DWORD)n, &written, NULL);
    CloseHandle(f);
    return (written == (DWORD)n) ? 0 : -4;
}