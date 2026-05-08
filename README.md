# mlibrary

Version 1.0.0

A Calibre-style ebook library desktop application built with Qt 6 C++.

## Features

- multi-library switching
- book grid and right-side Details / Preview panels
- add/edit metadata
- bulk metadata edit
- download metadata and cover art from the internet
- copy or move books between libraries
- modify subject
- file conversion dialog
- recursive website mirroring to PDF workflow
- built-in book viewer for text, HTML, PDF, images, and best-effort conversion-based preview for other formats
- PostgreSQL / MariaDB / MySQL / SQLite / Oracle database setup dialog
- permanent delete and duplicate merge scaffolding
- browser-accessible content server with approval workflow

## Book viewer

The viewer opens the selected book path and renders:
- txt
- html / htm
- xhtml
- xml
- md
- pdf
- common images such as png, jpg, jpeg, gif, bmp, webp, svg

For other common ebook formats such as epub, mobi, azw, azw3, dvi, cbr, cbz, and cdr, the viewer attempts a best-effort conversion through Calibre `ebook-convert` before opening the preview.

## Database backends

Supported backend types:
- PostgreSQL
- MariaDB / MySQL
- SQLite
- Oracle
- JSON Flatfile

Qt’s SQL docs say SQLite has the best in-process support on all platforms, while Oracle, PostgreSQL, and MySQL depend on the availability and quality of the matching client libraries. The docs also note that a driver plugin needs the appropriate client library for that DBMS. citeturn303923search0turn668667search2

## Configure the backend before running setup

Open **Setup DB** in the app and select the backend.

### PostgreSQL
Required:
- PostgreSQL client libraries
- Qt `QPSQL` plugin
- server host, port, database, user, password

Connection string example:
```text
host=localhost port=5432 dbname=ebook_library user=postgres password=postgres
```

### MariaDB / MySQL
Required:
- MariaDB/MySQL client libraries
- Qt `QMYSQL` plugin
- server host, port, database, user, password

Connection string example:
```text
host=localhost port=3306 dbname=ebook_library user=root password=secret
```

### SQLite
Required:
- Qt `QSQLITE` plugin

Provide:
- writable `.db` file path

Example:
```text
/home/user/mlibrary.db
```

### Oracle
Required:
- Oracle OCI client libraries
- Qt `QOCI` plugin
- `ORACLE_HOME` if your client install requires it
- `TNS_ADMIN` if you use tnsnames.ora
- service name or TNS alias if your environment uses one

Provide either:
- host/port/service connection details, or
- a TNS alias / service path if your Oracle installation uses that style

### JSON Flatfile
Required:
- no SQL server
- writable JSON file location
- a path you can create and reopen later

Provide:
- full path to the JSON flatfile, such as:
```text
/home/user/mlibrary.json
```

## Build prerequisites

You need:
- Qt 6
- CMake 3.21 or newer
- a C++17 toolchain
- the appropriate database client libraries for the backend you choose
- optional external tools:
  - Calibre `ebook-convert`
  - GNU Wget

## macOS M4 setup

### Option A: Qt Online Installer
1. Install Qt 6 using the Qt Online Installer.
2. Install Xcode Command Line Tools.
3. Open the project folder in Qt Creator.
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

Qt’s PDF module includes `QPdfView`, which Qt documents as a complete PDF viewer widget. To build CMake projects with Qt PDF, link the Qt PDF module. citeturn303923search1turn988588search2turn988588search8

## Linux setup

Example for Oracle Linux:
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++ postgresql-devel wget
```

If you plan to use MariaDB/MySQL or Oracle as the backend, install the matching client libraries and Qt driver plugin support as well. Qt’s SQL docs require the relevant client libraries for those database drivers. citeturn303923search0turn668667search2

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
- the database client libraries for your chosen backend

Open the folder in Qt Creator or Visual Studio with CMake support, then build the `EbookLibraryQt` target.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

On macOS:
```bash
./build/EbookLibraryQt.app/Contents/MacOS/EbookLibraryQt
```

On Linux/Windows:
```bash
./build/EbookLibraryQt
```

## Database setup

Use the **Setup DB** button inside the app to choose the backend and create the schema.

Before clicking **Create Tables**, make sure:
- the client libraries for the selected backend are installed
- the Qt driver plugin for that backend is available
- the connection path or service name is configured correctly
- the SQLite file path is writable if SQLite is selected
- the JSON flatfile path is writable if JSON Flatfile is selected
- the Oracle service / TNS settings are present if Oracle is selected

## Metadata lookup

Metadata download is wired to internet lookup services:
- Open Library Books API
- Open Library Covers API

## Conversion features

The conversion dialog is wired to Calibre's `ebook-convert` CLI.

Supported target formats in the UI include:
- PDF
- EPUB
- MOBI
- TXT
- HTML

## Website mirroring

The website import dialog uses GNU Wget for recursive site mirroring, then converts mirrored HTML to PDF.

## Content server

The app includes a built-in browser-accessible content server.

It exposes:
- Browse Library
- Download books
- Upload books by request, with admin approval
- Edit metadata by request, with admin approval

It does **not** expose delete-book or delete-library actions.

Start it from the toolbar using **Start Web Server**.

The port is configurable in the dialog and the last used port is remembered.
The admin email address is configurable as well.

Default local URL:
```text
http://127.0.0.1:8088
```

## Deployment

### macOS
Use `macdeployqt` after build:
```bash
macdeployqt build/EbookLibraryQt.app
```

### Windows
Use `windeployqt` on the built executable to collect Qt DLLs and plugins.

### Linux
Use AppImage, DEB, or RPM packaging. For Oracle Linux, RPM packaging is a good default.

## License
GPL v3.0


## Added application-level dialogs

The app now also includes:
- About dialog with owner / version details
- GPL v3.0 license dialog
- Keyboard shortcuts dialog
- Preferences dialog with database profiles, email settings, and web server security settings

## Notes on authentication and HTTPS

The built-in content server now supports optional HTTP basic authentication using configured credentials.

HTTPS settings are stored in Preferences as part of the configuration workflow, while the server currently remains an HTTP listener. TLS wiring can be added later if you want the server to terminate SSL directly inside the app.


## Search engine

The app now includes an advanced search dialog that can search:
- title
- authors
- subject
- tags
- description
- remarks
- publisher
- storage path

It supports:
- current library search
- all libraries search
- favorites-only filtering
- exact phrase matching
- ranked results

## AI authoring helper

The app now also includes an AI authoring helper for:
- synopsis
- back-cover copy
- author bio
- chapter outline
- series pitch
- improvement suggestions

It can work in two modes:
- local fallback templates, with no external service required
- remote AI endpoint mode, using a configurable OpenAI-compatible HTTP endpoint

### AI helper configuration

Open **Preferences** and set:
- endpoint URL
- API key
- model name
- system prompt

If remote mode is off or the endpoint is blank, the helper uses the local fallback generator.

## JSON flatfile backend

When you choose JSON Flatfile, the app creates a metadata document on disk and uses that path as the storage location. The file is a single structured JSON document containing libraries, books, tags, subjects, publishers, series, and related link tables.


## Single-user vs multi-user mode

During **Setup DB**, choose whether authentication is enabled.

- **Single-user mode**: no authentication, no user/role workflow, suitable for a personal laptop
- **Multi-user mode**: creates users, roles, permissions, sessions, and audit tables for shared or enterprise use

The built-in web server follows that setting by default, so a laptop install can remain simple while enterprise deployments can turn authentication on.


## Physical library support

The app now also supports cataloging physical books.

You can track:
- physical location
- section or room
- shelf or rack
- checked-out status
- borrower name and contact
- checkout date
- due date
- accession number
- barcode
- loan notes

This can be used for:
- home libraries
- personal collections
- office reference shelves
- enterprise physical archives
- hybrid libraries that mix digital files and print copies

A dedicated **Physical Copy** dialog lets you update the selected book’s physical inventory details.


Official website: https://www.adyainfotech.com
