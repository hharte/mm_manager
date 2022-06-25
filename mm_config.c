/*
 * Configuration module for mm_manager.
 *
 * Stores call accounting and statistics data from the
 * Nortel Millennium payphone in an sqlite3 or MariaDB
 * database.
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2022, Howard M. Harte
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./mm_manager.h"

#define TERMTYP_CSV_FNAME    "config/control_rom_versions.csv"
#define TELCO_ID_REGION_CODE "\"%c%c\",\"%c%c%c\""

#ifdef MYSQL_DB
#define AUTO_INCREMENT  "AUTO_INCREMENT"
#define SQL_IGNORE      "IGNORE "
#else
#define AUTO_INCREMENT "AUTOINCREMENT"
#define SQL_IGNORE      ""
#endif /* MYSQL */

uint8_t mm_config_get_term_type_from_control_rom_edition(void* db, const char* control_rom_edition) {
    char sql[256] = { 0 };
    char db_control_rom_edition[8];

    snprintf(db_control_rom_edition, sizeof(db_control_rom_edition), "%s", control_rom_edition);

    snprintf(sql, sizeof(sql), "SELECT TERMINAL_TYPE from TERMTYP where (CONTROL_ROM_EDITION = \"%s\"); ",
        db_control_rom_edition);

    return mm_sql_read_uint8(db, sql);
}

int mm_config_add_TERMTYP_entry(void *db, uint8_t terminal_type, const char *control_rom_edition, const char *description) {
    char sql[256] = { 0 };
    char db_control_rom_edition[8];
    char db_description[41];

    snprintf(db_control_rom_edition, sizeof(db_control_rom_edition), "%s", control_rom_edition);
    snprintf(db_description, sizeof(db_description), "%s", description);

    snprintf(sql, sizeof(sql), "INSERT " SQL_IGNORE "INTO TERMTYP ( "
        "TERMINAL_TYPE,"
        "CONTROL_ROM_EDITION,"
        "DESCRIPTION"
        " ) VALUES ( "
        "%d,\"%s\",\"%s\");",
        terminal_type,
        db_control_rom_edition,
        db_description);

    return mm_sql_exec(db, sql);
}

int mm_config_create_tables(void *db) {
    int rc;
    FILE* csvstream = NULL;
    char csvline[255] = { 0 };
    char* control_rom_edition = NULL;
    uint8_t terminal_type = 0;
    char* description = NULL;
    int line = 0;
    const char *tokens = ",";

    rc = mm_sql_exec(db, "CREATE TABLE IF NOT EXISTS TERMTYP ( "
        "ID INTEGER NOT NULL PRIMARY KEY " AUTO_INCREMENT ","
        "TERMINAL_TYPE TINYINT NOT NULL, "
        "CONTROL_ROM_EDITION VARCHAR(7),"
        "DESCRIPTION VARCHAR(40),"
        "UNIQUE(CONTROL_ROM_EDITION) "
        ");");

    if (rc != 0) {
        fprintf(stderr, "%s: Failed to create table TERMTYP.\n", __FUNCTION__);
        return -1;
    }

    if (!(csvstream = fopen(TERMTYP_CSV_FNAME, "r"))) {
        fprintf(stderr, "Error opening input stream: %s\n", TERMTYP_CSV_FNAME);
        return -EPERM;
    }

    while (!feof(csvstream)) {
        char* tok;
        fgets(csvline, sizeof(csvline), csvstream);
        line++;
        if (line == 1) continue; /* Skip over CSV header */

        control_rom_edition = strtok(csvline, tokens);
        tok = strtok(NULL, tokens);
        if (tok == NULL) break;
        terminal_type = atoi(tok);
        description = strtok(NULL, tokens);

        mm_config_add_TERMTYP_entry(db, terminal_type, control_rom_edition, description);
    }

    fclose(csvstream);

    return 0;
}
