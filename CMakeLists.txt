cmake_minimum_required(VERSION 3.21)
project(EbookLibraryQt VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Pdf PdfWidgets)

qt_add_executable(EbookLibraryQt
    src/main.cpp
    src/Book.h
    src/LibraryData.h
    src/DatabaseManager.h
    src/DatabaseManager.cpp
    src/MetadataService.h
    src/MetadataService.cpp
    src/AddLibraryDialog.h
    src/AddLibraryDialog.cpp
    src/AddBookDialog.h
    src/AddBookDialog.cpp
    src/DatabaseSetupDialog.h
    src/DatabaseSetupDialog.cpp
    src/MetadataDownloadDialog.h
    src/MetadataDownloadDialog.cpp
    src/BulkEditDialog.h
    src/BulkEditDialog.cpp
    src/CopyMoveDialog.h
    src/CopyMoveDialog.cpp
    src/SubjectEditDialog.h
    src/SubjectEditDialog.cpp
    src/ConversionDialog.h
    src/ConversionDialog.cpp
    src/WebsiteImportDialog.h
    src/WebsiteImportDialog.cpp
    src/WebServerService.h
    src/WebServerService.cpp
    src/WebServerDialog.h
    src/WebServerDialog.cpp
    src/BookViewerDialog.h
    src/BookViewerDialog.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(EbookLibraryQt PRIVATE Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Pdf Qt6::PdfWidgets)
target_include_directories(EbookLibraryQt PRIVATE src)
