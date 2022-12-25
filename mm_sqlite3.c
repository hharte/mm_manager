/*
 * SQLite3 Interface for mm_manager.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./mm_manager.h"

#define AUTO_INCREMENT "AUTOINCREMENT"

int mm_sql_exec(void *db, const char *sql) {
    int rc;

//    printf("SQL:\n%s\n", sql);
    rc = sqlite3_exec((sqlite3 *)db, sql, NULL, 0, NULL);

    if (rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
        fprintf(stderr, "%s: Failed to execute: \nSQL: '%s'\nError: %s", __func__, sql, sqlite3_errmsg((sqlite3 *)db));
        return -1;
    }

    return 0;
}

uint8_t mm_sql_read_uint8(void* db, const char* sql) {
    sqlite3_stmt* res = NULL;
    int rc = sqlite3_prepare_v2((sqlite3*)db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s: Failed to prepare: \nSQL: '%s'\nError: %s", __func__, sql, sqlite3_errmsg((sqlite3*)(db)));
        return 0xFF;
    }

    sqlite3_step(res);

    return (uint8_t)sqlite3_column_int(res, 0);
}

uint64_t mm_sql_read_uint64(void* db, const char* sql) {
    sqlite3_stmt* res = NULL;
    int rc = sqlite3_prepare_v2((sqlite3*)db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s: Failed to prepare: \nSQL: '%s'\nError: %s", __func__, sql, sqlite3_errmsg((sqlite3 *)(db)));
        return 0xFFFFFFFFFFFFFFFFLL;
    }

    sqlite3_step(res);

    return (uint64_t)sqlite3_column_int64(res, 0);
}

int mm_sql_write_blob(void* db, const char* sql, uint8_t *buffer, size_t buflen) {
    sqlite3_stmt* res = NULL;
    int rc;

    printf("SQL:\n%s\n", sql);
    rc = sqlite3_prepare_v2((sqlite3 *)db, sql, -1, &res, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg((sqlite3 *)db));
    }
    else {
        rc = sqlite3_bind_blob(res, 1, buffer, (int)buflen, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "bind failed: %s\n", sqlite3_errmsg((sqlite3*)db));
        }
        else {
            rc = sqlite3_step(res);
            if (rc != SQLITE_DONE) {
                fprintf(stderr, "execution failed: %s\n", sqlite3_errmsg((sqlite3*)db));
                return 0;
            }
        }
    }

    sqlite3_finalize(res);

    return (rc == SQLITE_DONE) ? 0 : rc;
}

int mm_sql_read_blob(void* db, const char* sql, uint8_t* buffer, size_t buflen) {
    sqlite3_stmt* res = NULL;
    int rc;
    printf("SQL:\n%s\n", sql);
    rc = sqlite3_prepare_v2((sqlite3*)db, sql, -1, &res, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "%s: Failed to prepare: \nSQL: '%s'\nError: %s", __func__, sql, sqlite3_errmsg((sqlite3*)(db)));
        return rc;
    }

    sqlite3_step(res);

    size_t blob_len = sqlite3_column_bytes(res, 0);
    if (blob_len > buflen) {
        fprintf(stderr, "%s: buffer length %zu is not large enough for table of %zu bytes.\n", __func__, buflen, blob_len);
        return 0;
    }

    memcpy(buffer, sqlite3_column_blob(res, 0), blob_len);

    return (int)blob_len;
}

int mm_sql_load_TCASHST(void* db, const char* terminal_id, cashbox_status_univ_t* cashbox_status) {
    int rc;
    const unsigned char* db_date_str;
    const unsigned char* db_time_str;
    char sql[512] = { 0 };
    uint16_t cashbox_year = 0;
    sqlite3_stmt* res;

    snprintf(sql, sizeof(sql), "SELECT START_DATE, printf(\"%%06d\", START_TIME) AS START_TIME, CASH_BOX_STATUS, PERCENT_FULL, CURRENCY_VALUE,"
        "NUMBER_OF_CDN_NICKELS, NUMBER_OF_CDN_DIMES, NUMBER_OF_CDN_QUARTERS, NUMBER_OF_CDN_DOLLARS,"
        "NUMBER_OF_US_NICKELS,  NUMBER_OF_US_DIMES,  NUMBER_OF_US_QUARTERS,  NUMBER_OF_US_DOLLARS "
        "from TCASHST where (TERMINAL_ID = \"%s\" )",
        terminal_id);

    rc = sqlite3_prepare_v2((sqlite3 *)db, sql, -1, &res, 0);

    if (rc != SQLITE_OK && rc != SQLITE_CONSTRAINT) {
        fprintf(stderr, "%s: Failed to prepare: \nSQL: '%s'\nError: %s", __func__, sql, sqlite3_errmsg((sqlite3 *)db));
        return 1;
    }

    sqlite3_step(res);

    memset(cashbox_status, 0, sizeof(cashbox_status_univ_t));
    cashbox_status->id = DLOG_MT_CASH_BOX_STATUS;

    db_date_str = sqlite3_column_text(res, 0);
    db_time_str = sqlite3_column_text(res, 1);

    if (db_date_str != NULL && db_time_str != NULL) {
        if (3 != sscanf((const char*)db_date_str, "%4hu%2hhu%2hhu",
            &cashbox_year,
            &cashbox_status->timestamp[1],
            &cashbox_status->timestamp[2])) {

            fprintf(stderr, "%s: Error parsing date string from database.\n", __func__);
        }
        cashbox_status->timestamp[0] = cashbox_year - 1900;

        if (3 != sscanf((const char*)db_time_str, "%2hhu%2hhu%2hhu",
            &cashbox_status->timestamp[3],
            &cashbox_status->timestamp[4],
            &cashbox_status->timestamp[5])) {

            fprintf(stderr, "%s: Error parsing time string from database.\n", __func__);
        }

        cashbox_status->status = sqlite3_column_int(res, 2);
        cashbox_status->percent_full = sqlite3_column_int(res, 3);
        cashbox_status->currency_value = (uint16_t)sqlite3_column_double(res, 4) * 100;
        cashbox_status->coin_count[COIN_COUNT_CA_NICKELS] = sqlite3_column_int(res, 5);
        cashbox_status->coin_count[COIN_COUNT_CA_DIMES] = sqlite3_column_int(res, 6);
        cashbox_status->coin_count[COIN_COUNT_CA_QUARTERS] = sqlite3_column_int(res, 7);
        cashbox_status->coin_count[COIN_COUNT_CA_DOLLARS] = sqlite3_column_int(res, 8);
        cashbox_status->coin_count[COIN_COUNT_US_NICKELS] = sqlite3_column_int(res, 9);
        cashbox_status->coin_count[COIN_COUNT_US_DIMES] = sqlite3_column_int(res, 10);
        cashbox_status->coin_count[COIN_COUNT_US_QUARTERS] = sqlite3_column_int(res, 11);
        cashbox_status->coin_count[COIN_COUNT_US_DOLLARS] = sqlite3_column_int(res, 12);
    } else {
        printf("Cannot retrieve cash box status, assuming empty.\n");
        cashbox_status->timestamp[0] = 90;
        cashbox_status->timestamp[1] = 1;
        cashbox_status->timestamp[2] = 1;
    }

    return 0;
}

int mm_open_database(mm_context_t *context) {
    sqlite3 *db = { 0 };

    int rc = sqlite3_open("mm_manager.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if (mm_acct_create_tables(db) != 0) {
        fprintf(stderr, "Failure creating accounting tables: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    if (mm_table_create_tables(db) != 0) {
        fprintf(stderr, "Failure creating data tables: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    if (mm_config_create_tables(db) != 0) {
        fprintf(stderr, "Failure creating configuration tables: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    context->database = db;
    return 0;
}

int mm_close_database(mm_context_t *context) {
    sqlite3 *db = (sqlite3 *)context->database;

    sqlite3_close(db);

    return 0;
}
