/* JajaDisk - Copyright (C) 2026 杨子桐 - AGPL-3.0 */
#pragma once
#ifdef JAJADISKCORE_EXPORTS
#define JAJA_API __declspec(dllexport)
#else
#define JAJA_API __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long long startLba;
    unsigned long long sizeBytes;
    int partitionNumber;
    int isGpt;
    int isActive;
    wchar_t label[64];
    wchar_t fsType[16];
} PartitionInfo;

typedef struct {
    wchar_t name[256];
    unsigned long long sizeBytes;
    int isDirectory;
    int isDeleted;
} FileInfo;

JAJA_API int JajaDisk_GetPartitions(const wchar_t* diskPath, PartitionInfo* outBuf, int maxCount);
JAJA_API int JajaDisk_ReadSector(const wchar_t* diskPath, unsigned long long lba, unsigned char* buf, int bufSize);
JAJA_API int JajaDisk_WriteSector(const wchar_t* diskPath, unsigned long long lba, const unsigned char* buf, int bufSize);
JAJA_API int JajaDisk_BackupPartitionTable(const wchar_t* diskPath, const wchar_t* outFile);
JAJA_API int JajaDisk_RestorePartitionTable(const wchar_t* diskPath, const wchar_t* inFile);
JAJA_API int JajaDisk_DeletePartition(const wchar_t* diskPath, int partitionNumber);
JAJA_API int JajaDisk_FormatVolume(const wchar_t* volumePath, const wchar_t* fsType);

// 新增：解析文件系统（基于挂载的盘符）
JAJA_API int JajaDisk_ListFiles(const wchar_t* driveLetter, FileInfo* outBuf, int maxCount);
// 新增：恢复被删除的文件（模拟）
JAJA_API int JajaDisk_RecoverFile(const wchar_t* driveLetter, const wchar_t* fileName, const wchar_t* savePath);

#ifdef __cplusplus
}
#endif