#include "DatabaseManager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

bool DatabaseManager::createSchema(QSqlDatabase &db, QString *errorMessage) {
    if (!db.isOpen() && !db.open()) {
        if (errorMessage) *errorMessage = db.lastError().text();
        return false;
    }

    const QStringList statements = {
        "CREATE EXTENSION IF NOT EXISTS pg_trgm;",
        "CREATE EXTENSION IF NOT EXISTS unaccent;",
        "CREATE TABLE IF NOT EXISTS libraries ("
        "id BIGSERIAL PRIMARY KEY, name TEXT NOT NULL UNIQUE, slug TEXT NOT NULL UNIQUE, root_path TEXT NOT NULL, "
        "is_active BOOLEAN NOT NULL DEFAULT FALSE, created_at TIMESTAMP DEFAULT NOW());",
        "CREATE TABLE IF NOT EXISTS book_types (id BIGSERIAL PRIMARY KEY, type_name TEXT NOT NULL UNIQUE);",
        "CREATE TABLE IF NOT EXISTS publishers (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), publisher_name TEXT NOT NULL, publisher_slug TEXT NOT NULL, country TEXT, created_at TIMESTAMP DEFAULT NOW(), UNIQUE(library_id, publisher_slug));",
        "CREATE TABLE IF NOT EXISTS series (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), series_name TEXT NOT NULL, series_slug TEXT NOT NULL, created_at TIMESTAMP DEFAULT NOW(), UNIQUE(library_id, series_slug));",
        "CREATE TABLE IF NOT EXISTS subjects (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), subject_name TEXT NOT NULL, subject_slug TEXT NOT NULL, parent_subject_id BIGINT REFERENCES subjects(id), created_at TIMESTAMP DEFAULT NOW(), UNIQUE(library_id, subject_slug));",
        "CREATE TABLE IF NOT EXISTS tags (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), tag_name TEXT NOT NULL, tag_slug TEXT NOT NULL, created_at TIMESTAMP DEFAULT NOW(), UNIQUE(library_id, tag_slug));",
        "CREATE TABLE IF NOT EXISTS books (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), title TEXT NOT NULL, subtitle TEXT, description TEXT, published_date DATE, added_date TIMESTAMP NOT NULL DEFAULT NOW(), remarks TEXT, is_favorite BOOLEAN NOT NULL DEFAULT FALSE, book_type_id BIGINT REFERENCES book_types(id), publisher_id BIGINT REFERENCES publishers(id), series_id BIGINT REFERENCES series(id), series_number TEXT, original_language VARCHAR(20), isbn10 TEXT, isbn13 TEXT, country TEXT, search_vector TSVECTOR, cover_path TEXT, created_at TIMESTAMP DEFAULT NOW(), updated_at TIMESTAMP DEFAULT NOW());",
        "CREATE TABLE IF NOT EXISTS book_contributors (id BIGSERIAL PRIMARY KEY, book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE, contributor_name TEXT NOT NULL, contributor_role TEXT NOT NULL);",
        "CREATE TABLE IF NOT EXISTS book_subjects (book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE, subject_id BIGINT NOT NULL REFERENCES subjects(id) ON DELETE CASCADE, is_primary BOOLEAN NOT NULL DEFAULT FALSE, PRIMARY KEY(book_id, subject_id));",
        "CREATE TABLE IF NOT EXISTS book_tags (book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE, tag_id BIGINT NOT NULL REFERENCES tags(id) ON DELETE CASCADE, PRIMARY KEY(book_id, tag_id));",
        "CREATE TABLE IF NOT EXISTS file_types (id BIGSERIAL PRIMARY KEY, library_id BIGINT NOT NULL REFERENCES libraries(id), extension TEXT NOT NULL, mime_type TEXT, created_at TIMESTAMP DEFAULT NOW(), UNIQUE(library_id, extension));",
        "CREATE TABLE IF NOT EXISTS book_files (id BIGSERIAL PRIMARY KEY, book_id BIGINT NOT NULL REFERENCES books(id) ON DELETE CASCADE, library_id BIGINT NOT NULL REFERENCES libraries(id), file_type_id BIGINT REFERENCES file_types(id), file_name TEXT NOT NULL, storage_path TEXT NOT NULL, checksum_sha256 TEXT NOT NULL, file_size BIGINT, created_at TIMESTAMP DEFAULT NOW());",
        "CREATE INDEX IF NOT EXISTS idx_books_search ON books USING GIN(search_vector);",
        "CREATE UNIQUE INDEX IF NOT EXISTS uq_books_library_isbn13 ON books(library_id, isbn13) WHERE isbn13 IS NOT NULL;"
    };

    for (const auto &sql : statements) {
        QSqlQuery q(db);
        if (!q.exec(sql)) {
            if (errorMessage) *errorMessage = q.lastError().text();
            return false;
        }
    }
    return true;
}
