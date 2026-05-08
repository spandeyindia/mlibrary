# mlibrary
<<<<<<< HEAD
ebook library using postgres-sql as backend db
=======

A Calibre-style ebook library desktop application built with Qt 6 C++ and PostgreSQL.

## What this repository contains

This repository is a desktop application starter for managing a large ebook library with:

- multi-library switching
- book grid and right-side Details / Preview panels
- add/edit metadata
- bulk metadata edit
- download metadata and cover art from the internet
- copy or move books between libraries
- modify subject
- file conversion dialog
- recursive website mirroring to PDF workflow
- PostgreSQL schema setup dialog
- permanent delete and duplicate merge scaffolding

## Repository layout

```text
.
├── CMakeLists.txt
├── README.md
├── docs/
│   └── schema.sql
└── src/
    ├── main.cpp
    ├── MainWindow.*
    ├── DatabaseSetupDialog.*
    ├── DatabaseManager.*
    ├── MetadataService.*
    ├── MetadataDownloadDialog.*
    ├── BulkEditDialog.*
    ├── CopyMoveDialog.*
    ├── SubjectEditDialog.*
    ├── ConversionService.*
    ├── ConversionDialog.*
    ├── WebsiteImportDialog.*
    ├── AddLibraryDialog.*
    └── AddBookDialog.*
```

## Prerequisites

You need:

- **Qt 6**
- **CMake 3.21 or newer**
- **C++17 toolchain**
- **PostgreSQL** client/server access
- Optional external tools used by some dialogs:
  - Calibre `ebook-convert`
  - GNU Wget

## macOS M4 setup

### Option A: Qt Online Installer
1. Install Qt 6 using the Qt Online Installer.
2. Install Xcode Command Line Tools.
3. Open the project folder in **Qt Creator**.
4. Configure a Qt 6 kit.
5. Build and run.

### Option B: Homebrew
Install dependencies:

```bash
brew install cmake qt postgresql wget
```

Then configure CMake using the Qt install prefix. On Apple Silicon this is typically under `/opt/homebrew`.

Example:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt
cmake --build build
./build/EbookLibraryQt.app/Contents/MacOS/EbookLibraryQt
```

## Linux setup

Install packages through your distribution package manager. For example on Oracle Linux:

```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++ postgresql-devel wget
```

Then build:

```bash
cmake -S . -B build
cmake --build build
./build/EbookLibraryQt
```

## Windows setup

Install:

- Qt 6
- CMake
- MSVC Build Tools
- PostgreSQL client libraries

Open the folder in Qt Creator or Visual Studio with CMake support, then build the `EbookLibraryQt` target.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/EbookLibraryQt
```

On macOS, the actual app bundle will be:

```bash
./build/EbookLibraryQt.app
```

## Database setup

Use the **Setup DB** button inside the app to create the PostgreSQL schema.

The schema includes:

- libraries
- books
- book_files
- book_tags
- book_subjects
- tags
- subjects
- publishers
- series
- file types
- contributors

## Metadata lookup

Metadata download is wired to internet lookup services:

- Open Library Books API
- Open Library Covers API

The UI can auto-fill metadata and cover images from ISBN lookups.

## Conversion features

The conversion dialog is wired to Calibre's `ebook-convert` CLI.

Supported target formats in the UI include:

- PDF
- EPUB
- MOBI
- TXT
- HTML

For best results, install Calibre on the machine where conversions will run.

## Website mirroring

The website import dialog uses GNU Wget for recursive site mirroring, then converts mirrored HTML to PDF.

## Packaging / deployment

### macOS
Use `macdeployqt` after build:

```bash
macdeployqt build/EbookLibraryQt.app
```

If you want a distributable disk image, wrap the app bundle in a DMG after deployment.

### Windows
Use `windeployqt` on the built `.exe` to collect Qt DLLs and plugins.

### Linux
Use your preferred packaging route:

- AppImage
- RPM
- DEB

For Oracle Linux, RPM packaging is usually the cleanest.

## Development notes

The app currently ships as a functional starter with:
- dialogs
- schema setup
- sample data
- grid and panels
- management workflows

The next natural steps are:
- persist all library data to PostgreSQL
- wire import/export actions into real storage
- connect copy/move operations to filesystem roots
- connect metadata edits to database writes
- connect conversion output back into book records

## License

Add your preferred license before publishing.
>>>>>>> df50915 (Initial ebook library app)
