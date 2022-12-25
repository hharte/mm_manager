/*
 * Table Management module for mm_manager.
 *
 * Database for table management.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./mm_manager.h"

#define TELCO_ID_REGION_CODE "\"%c%c\",\"%c%c%c\""

#ifdef MYSQL_DB
#define AUTO_INCREMENT  "AUTO_INCREMENT"
#define SQL_IGNORE      "IGNORE "
#else
#define AUTO_INCREMENT "AUTOINCREMENT"
#define SQL_IGNORE      ""
#endif /* MYSQL */


size_t mm_table_load(mm_context_t *context, uint8_t table_id, uint64_t version_timestamp, uint8_t *buffer, size_t buflen) {
    char sql[512] = { 0 };
    size_t blob_len;

    snprintf(sql, sizeof(sql), "SELECT TABLE_DATA from TERMDAT where (TABLE_ID = %d and VERSION_TIMESTAMP = %" PRIu64 ");",
        table_id, version_timestamp);

    blob_len = mm_sql_read_blob(context->database, sql, buffer, buflen);

    return blob_len;
}

int mm_table_save(mm_context_t* context, uint8_t table_id, uint64_t version_timestamp, uint8_t* buffer, size_t buflen) {
    char sql[512] = { 0 };
    int rc;

    snprintf(sql, sizeof(sql), "REPLACE INTO TERMDAT (TABLE_ID, VERSION_TIMESTAMP, DATA_LENGTH, TABLE_DATA)"
        "VALUES ( %d, %" PRIu64 ", %zd, ?);",
        table_id,
        version_timestamp,
        buflen);

    buffer[0] = table_id;

    rc = mm_sql_write_blob(context->database, sql, buffer, buflen);

    if (rc != 0) {
        fprintf(stderr, "%s: Error writing table %d, version %" PRIu64 "\n", __func__, table_id, version_timestamp);
    }

    return rc;
}

int mm_table_create_tables(void *db) {
    int rc;

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TERMDAT ( "
        "TABLE_ID INTEGER NOT NULL,"
        "VERSION_TIMESTAMP TIMESTAMP NOT NULL,"
        "DATA_LENGTH SMALLINT NOT NULL,"
        "TABLE_DATA BLOB,"
        " PRIMARY KEY(TABLE_ID, VERSION_TIMESTAMP),"
        "UNIQUE(TABLE_ID,VERSION_TIMESTAMP));");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TERMDAT.\n", __func__);
        return -1;
    }

    return 0;
}
