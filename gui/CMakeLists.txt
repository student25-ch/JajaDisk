add_executable(JajaDisk
    main.cpp
    MainWindow.cpp
    MainWindow.h
    PartitionMapWidget.cpp
    PartitionMapWidget.h
    FileBrowserWidget.cpp   # 如果你加了，确保在这
    FileBrowserWidget.h
)

target_include_directories(JajaDisk PRIVATE ${CMAKE_SOURCE_DIR}/core)

# 这一行必须存在，且必须是 Qt6::Widgets
target_link_libraries(JajaDisk PRIVATE Qt6::Widgets JajaDiskCore)