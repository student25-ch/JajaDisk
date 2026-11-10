# JajaDisk - 开源磁盘管理与数据恢复工具
# Copyright (C) 2026 杨子桐
# 许可证: AGPL-3.0-or-later

add_library(JajaDiskCore SHARED
    JajaDiskCore.cpp
    JajaDiskCore.h
)



# 强制将 DLL 统一输出到构建目录的根下（和 exe 在一起）
set_target_properties(JajaDiskCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)