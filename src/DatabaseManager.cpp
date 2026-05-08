#include "DatabaseManager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

QString DatabaseManager::backendKey(DbBackend backend) {
    switch (backend) {
        case DbBackend::PostgreSQL: return "PostgreSQL";
        case DbBackend::MariaDB: return "MariaDB/MySQL";
        case DbBackend::SQLite: return "SQLite";
        case DbBackend::Oracle: return "Oracle";
    }
    return "PostgreSQL";
}

QString DatabaseManager::driverForBackend(DbBackend backend) {
    switch (backend) {
        case DbBackend::PostgreSQL: return "QPSQL";
        case DbBackend::MariaDB: return "QMYSQL";
        case DbBackend::SQLite: return "QSQLITE";
        case DbBackend::Oracle: return "QOCI";
    }
    return "QPSQL";
}

static QString schemaPostgres() {
    return R"SQL(
CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE EXTENSION IF NOT EXISTS unaccent;

CREATE TABLE IF NOT EXISTS libraries (
    id BIGSERIAL PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,
    slug TEXT NOT NULL UNIQUE,
    root_path TEXT NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS book_types (
    id BIGSERIAL PRIMARY KEY,
    type_name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS publishers (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    publisher_name TEXT NOT NULL,
    publisher_slug TEXT NOT NULL,
    country TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(library_id, publisher_slug)
);

CREATE TABLE IF NOT EXISTS series (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    series_name TEXT NOT NULL,
    series_slug TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(library_id, series_slug)
);

CREATE TABLE IF NOT EXISTS subjects (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    subject_name TEXT NOT NULL,
    subject_slug TEXT NOT NULL,
    parent_subject_id BIGINT REFERENCES subjects(id),
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(library_id, subject_slug)
);

CREATE TABLE IF NOT EXISTS tags (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    tag_name TEXT NOT NULL,
    tag_slug TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(library_id, tag_slug)
);

CREATE TABLE IF NOT EXISTS books (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    title TEXT NOT NULL,
    subtitle TEXT,
    description TEXT,
    published_date DATE,
    added_date TIMESTAMP NOT NULL DEFAULT NOW(),
    remarks TEXT,
    is_favorite BOOLEAN NOT NULL DEFAULT FALSE,
    book_type_id BIGINT REFERENCES book_types(id),
    publisher_id BIGINT REFERENCES publishers(id),
    series_id BIGINT REFERENCES series(id),
    series_number TEXT,
    original_language VARCHAR(20),
    isbn10 TEXT,
    isbn13 TEXT,
    country TEXT,
    search_vector TSVECTOR,
    cover_path TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS book_contributors (
    id BIGSERIAL PRIMARY KEY,
    book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE,
    contributor_name TEXT NOT NULL,
    contributor_role TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS book_subjects (
    book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE,
    subject_id BIGINT NOT NULL REFERENCES subjects(id) ON DELETE CASCADE,
    is_primary BOOLEAN NOT NULL DEFAULT FALSE,
    PRIMARY KEY(book_id, subject_id)
);

CREATE TABLE IF NOT EXISTS book_tags (
    book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE,
    tag_id BIGINT NOT NULL REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY(book_id, tag_id)
);

CREATE TABLE IF NOT EXISTS file_types (
    id BIGSERIAL PRIMARY KEY,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    extension TEXT NOT NULL,
    mime_type TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(library_id, extension)
);

CREATE TABLE IF NOT EXISTS book_files (
    id BIGSERIAL PRIMARY KEY,
    book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE,
    library_id BIGINT NOT NULL REFERENCES libraries(id),
    file_type_id BIGINT REFERENCES file_types(id),
    file_name TEXT NOT NULL,
    storage_path TEXT NOT NULL,
    checksum_sha256 TEXT NOT NULL,
    file_size BIGINT,
    created_at TIMESTAMP DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_books_search ON books USING GIN(search_vector);
CREATE UNIQUE INDEX IF NOT EXISTS uq_books_library_isbn13 ON books(library_id, isbn13) WHERE isbn13 IS NOT NULL;
)SQL";
}

static QString schemaMariaDb() {
    return R"SQL(
CREATE TABLE IF NOT EXISTS libraries (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(255) NOT NULL UNIQUE,
    slug VARCHAR(255) NOT NULL UNIQUE,
    root_path TEXT NOT NULL,
    is_active BOOLEAN NOT NULL DEFAULT FALSE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS book_types (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    type_name VARCHAR(255) NOT NULL UNIQUE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS publishers (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    publisher_name VARCHAR(255) NOT NULL,
    publisher_slug VARCHAR(255) NOT NULL,
    country VARCHAR(255),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_publishers_library_slug (library_id, publisher_slug),
    CONSTRAINT fk_publishers_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS series (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    series_name VARCHAR(255) NOT NULL,
    series_slug VARCHAR(255) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_series_library_slug (library_id, series_slug),
    CONSTRAINT fk_series_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS subjects (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    subject_name VARCHAR(255) NOT NULL,
    subject_slug VARCHAR(255) NOT NULL,
    parent_subject_id BIGINT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_subjects_library_slug (library_id, subject_slug),
    CONSTRAINT fk_subjects_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS tags (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    tag_name VARCHAR(255) NOT NULL,
    tag_slug VARCHAR(255) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_tags_library_slug (library_id, tag_slug),
    CONSTRAINT fk_tags_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS books (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    title VARCHAR(1024) NOT NULL,
    subtitle VARCHAR(1024),
    description TEXT,
    published_date DATE,
    added_date DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    remarks TEXT,
    is_favorite BOOLEAN NOT NULL DEFAULT FALSE,
    book_type_id BIGINT NULL,
    publisher_id BIGINT NULL,
    series_id BIGINT NULL,
    series_number VARCHAR(255),
    original_language VARCHAR(20),
    isbn10 VARCHAR(32),
    isbn13 VARCHAR(32),
    country VARCHAR(255),
    cover_path TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    KEY idx_books_isbn13 (isbn13),
    CONSTRAINT fk_books_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS book_contributors (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    book_id BIGINT NOT NULL,
    contributor_name VARCHAR(255) NOT NULL,
    contributor_role VARCHAR(255) NOT NULL,
    CONSTRAINT fk_book_contributors_book FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS book_subjects (
    book_id BIGINT NOT NULL,
    subject_id BIGINT NOT NULL,
    is_primary BOOLEAN NOT NULL DEFAULT FALSE,
    PRIMARY KEY(book_id, subject_id),
    CONSTRAINT fk_book_subjects_book FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    CONSTRAINT fk_book_subjects_subject FOREIGN KEY (subject_id) REFERENCES subjects(id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS book_tags (
    book_id BIGINT NOT NULL,
    tag_id BIGINT NOT NULL,
    PRIMARY KEY(book_id, tag_id),
    CONSTRAINT fk_book_tags_book FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    CONSTRAINT fk_book_tags_tag FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS file_types (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    library_id BIGINT NOT NULL,
    extension VARCHAR(32) NOT NULL,
    mime_type VARCHAR(255),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_file_types_library_ext (library_id, extension),
    CONSTRAINT fk_file_types_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS book_files (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    book_id BIGINT NOT NULL,
    library_id BIGINT NOT NULL,
    file_type_id BIGINT NULL,
    file_name VARCHAR(1024) NOT NULL,
    storage_path TEXT NOT NULL,
    checksum_sha256 VARCHAR(128) NOT NULL,
    file_size BIGINT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_book_files_book FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    CONSTRAINT fk_book_files_library FOREIGN KEY (library_id) REFERENCES libraries(id)
) ENGINE=InnoDB;
)SQL";
}

static QString schemaSQLite() {
    return R"SQL(
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS libraries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    slug TEXT NOT NULL UNIQUE,
    root_path TEXT NOT NULL,
    is_active INTEGER NOT NULL DEFAULT 0,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS book_types (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type_name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS publishers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    publisher_name TEXT NOT NULL,
    publisher_slug TEXT NOT NULL,
    country TEXT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(library_id, publisher_slug),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS series (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    series_name TEXT NOT NULL,
    series_slug TEXT NOT NULL,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(library_id, series_slug),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS subjects (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    subject_name TEXT NOT NULL,
    subject_slug TEXT NOT NULL,
    parent_subject_id INTEGER,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(library_id, subject_slug),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    tag_name TEXT NOT NULL,
    tag_slug TEXT NOT NULL,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(library_id, tag_slug),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS books (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    subtitle TEXT,
    description TEXT,
    published_date TEXT,
    added_date TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    remarks TEXT,
    is_favorite INTEGER NOT NULL DEFAULT 0,
    book_type_id INTEGER,
    publisher_id INTEGER,
    series_id INTEGER,
    series_number TEXT,
    original_language TEXT,
    isbn10 TEXT,
    isbn13 TEXT,
    country TEXT,
    cover_path TEXT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    updated_at TEXT DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS book_contributors (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    book_id INTEGER NOT NULL,
    contributor_name TEXT NOT NULL,
    contributor_role TEXT NOT NULL,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS book_subjects (
    book_id INTEGER NOT NULL,
    subject_id INTEGER NOT NULL,
    is_primary INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY(book_id, subject_id),
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    FOREIGN KEY (subject_id) REFERENCES subjects(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS book_tags (
    book_id INTEGER NOT NULL,
    tag_id INTEGER NOT NULL,
    PRIMARY KEY(book_id, tag_id),
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS file_types (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    library_id INTEGER NOT NULL,
    extension TEXT NOT NULL,
    mime_type TEXT,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(library_id, extension),
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);

CREATE TABLE IF NOT EXISTS book_files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    book_id INTEGER NOT NULL,
    library_id INTEGER NOT NULL,
    file_type_id INTEGER,
    file_name TEXT NOT NULL,
    storage_path TEXT NOT NULL,
    checksum_sha256 TEXT NOT NULL,
    file_size INTEGER,
    created_at TEXT DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    FOREIGN KEY (library_id) REFERENCES libraries(id)
);
)SQL";
}

static QString schemaOracle() {
    return R"SQL(
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE libraries (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        name VARCHAR2(255) NOT NULL UNIQUE,
        slug VARCHAR2(255) NOT NULL UNIQUE,
        root_path CLOB NOT NULL,
        is_active NUMBER(1) DEFAULT 0 NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE book_types (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        type_name VARCHAR2(255) NOT NULL UNIQUE
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE publishers (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        publisher_name VARCHAR2(255) NOT NULL,
        publisher_slug VARCHAR2(255) NOT NULL,
        country VARCHAR2(255),
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE (library_id, publisher_slug)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE series (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        series_name VARCHAR2(255) NOT NULL,
        series_slug VARCHAR2(255) NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE (library_id, series_slug)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE subjects (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        subject_name VARCHAR2(255) NOT NULL,
        subject_slug VARCHAR2(255) NOT NULL,
        parent_subject_id NUMBER,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE (library_id, subject_slug)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE tags (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        tag_name VARCHAR2(255) NOT NULL,
        tag_slug VARCHAR2(255) NOT NULL,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE (library_id, tag_slug)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE books (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        title VARCHAR2(1024) NOT NULL,
        subtitle VARCHAR2(1024),
        description CLOB,
        published_date DATE,
        added_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL,
        remarks CLOB,
        is_favorite NUMBER(1) DEFAULT 0 NOT NULL,
        book_type_id NUMBER,
        publisher_id NUMBER,
        series_id NUMBER,
        series_number VARCHAR2(255),
        original_language VARCHAR2(20),
        isbn10 VARCHAR2(32),
        isbn13 VARCHAR2(32),
        country VARCHAR2(255),
        cover_path CLOB,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE book_contributors (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        book_id NUMBER NOT NULL,
        contributor_name VARCHAR2(255) NOT NULL,
        contributor_role VARCHAR2(255) NOT NULL
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE book_subjects (
        book_id NUMBER NOT NULL,
        subject_id NUMBER NOT NULL,
        is_primary NUMBER(1) DEFAULT 0 NOT NULL,
        PRIMARY KEY(book_id, subject_id)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE book_tags (
        book_id NUMBER NOT NULL,
        tag_id NUMBER NOT NULL,
        PRIMARY KEY(book_id, tag_id)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE file_types (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        library_id NUMBER NOT NULL,
        extension VARCHAR2(32) NOT NULL,
        mime_type VARCHAR2(255),
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        UNIQUE (library_id, extension)
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
BEGIN
    EXECUTE IMMEDIATE 'CREATE TABLE book_files (
        id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY,
        book_id NUMBER NOT NULL,
        library_id NUMBER NOT NULL,
        file_type_id NUMBER,
        file_name VARCHAR2(1024) NOT NULL,
        storage_path CLOB NOT NULL,
        checksum_sha256 VARCHAR2(128) NOT NULL,
        file_size NUMBER,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )';
EXCEPTION WHEN OTHERS THEN NULL; END;
/
)SQL";
}

QString DatabaseManager::schemaSql(DbBackend backend) {
    switch (backend) {
        case DbBackend::PostgreSQL: return schemaPostgres();
        case DbBackend::MariaDB: return schemaMariaDb();
        case DbBackend::SQLite: return schemaSQLite();
        case DbBackend::Oracle: return schemaOracle();
    }
    return schemaPostgres();
}

static QStringList statementsForBackend(DbBackend backend) {
    QString schema = DatabaseManager::schemaSql(backend);
    QStringList statements;

    if (backend == DbBackend::Oracle) {
        // Oracle schema is intentionally best-effort and executed statement-by-statement in the dialog.
        const QStringList blocks = {
            "CREATE TABLE libraries ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, name VARCHAR2(255) NOT NULL UNIQUE, slug VARCHAR2(255) NOT NULL UNIQUE, root_path CLOB NOT NULL, is_active NUMBER(1) DEFAULT 0 NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP )",
            "CREATE TABLE book_types ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, type_name VARCHAR2(255) NOT NULL UNIQUE )",
            "CREATE TABLE publishers ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, publisher_name VARCHAR2(255) NOT NULL, publisher_slug VARCHAR2(255) NOT NULL, country VARCHAR2(255), created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE (library_id, publisher_slug) )",
            "CREATE TABLE series ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, series_name VARCHAR2(255) NOT NULL, series_slug VARCHAR2(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE (library_id, series_slug) )",
            "CREATE TABLE subjects ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, subject_name VARCHAR2(255) NOT NULL, subject_slug VARCHAR2(255) NOT NULL, parent_subject_id NUMBER, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE (library_id, subject_slug) )",
            "CREATE TABLE tags ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, tag_name VARCHAR2(255) NOT NULL, tag_slug VARCHAR2(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE (library_id, tag_slug) )",
            "CREATE TABLE books ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, title VARCHAR2(1024) NOT NULL, subtitle VARCHAR2(1024), description CLOB, published_date DATE, added_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, remarks CLOB, is_favorite NUMBER(1) DEFAULT 0 NOT NULL, book_type_id NUMBER, publisher_id NUMBER, series_id NUMBER, series_number VARCHAR2(255), original_language VARCHAR2(20), isbn10 VARCHAR2(32), isbn13 VARCHAR2(32), country VARCHAR2(255), cover_path CLOB, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP )",
            "CREATE TABLE book_contributors ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, book_id NUMBER NOT NULL, contributor_name VARCHAR2(255) NOT NULL, contributor_role VARCHAR2(255) NOT NULL )",
            "CREATE TABLE book_subjects ( book_id NUMBER NOT NULL, subject_id NUMBER NOT NULL, is_primary NUMBER(1) DEFAULT 0 NOT NULL, PRIMARY KEY(book_id, subject_id) )",
            "CREATE TABLE book_tags ( book_id NUMBER NOT NULL, tag_id NUMBER NOT NULL, PRIMARY KEY(book_id, tag_id) )",
            "CREATE TABLE file_types ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, library_id NUMBER NOT NULL, extension VARCHAR2(32) NOT NULL, mime_type VARCHAR2(255), created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, UNIQUE (library_id, extension) )",
            "CREATE TABLE book_files ( id NUMBER GENERATED BY DEFAULT AS IDENTITY PRIMARY KEY, book_id NUMBER NOT NULL, library_id NUMBER NOT NULL, file_type_id NUMBER, file_name VARCHAR2(1024) NOT NULL, storage_path CLOB NOT NULL, checksum_sha256 VARCHAR2(128) NOT NULL, file_size NUMBER, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP )"
        };
        return blocks;
    }

    // For the other DBs split on semicolon.
    for (const QString &stmt : schema.split(';', Qt::SkipEmptyParts)) {
        const QString sql = stmt.trimmed();
        if (!sql.isEmpty()) statements << sql;
    }
    return statements;
}

bool DatabaseManager::createSchema(QSqlDatabase &db, DbBackend backend, QString *errorMessage) {
    if (!db.isOpen() && !db.open()) {
        if (errorMessage) *errorMessage = db.lastError().text();
        return false;
    }

    const auto statements = statementsForBackend(backend);
    for (const QString &sql : statements) {
        QSqlQuery q(db);
        if (!q.exec(sql)) {
            if (errorMessage) *errorMessage = q.lastError().text() + " | SQL: " + sql.left(180);
            return false;
        }
    }
    return true;
}
