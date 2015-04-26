#include "mysqlc.h"
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "sh_log.h"

struct mysqlc {
    MYSQL *M;
};

#define MYSQL_ERROR(self, sql) \
    sh_error("Mysql exec `%s` error: %s\n", sql, mysql_error(self->M));

struct mysqlc *
mysqlc_create(const struct mysqlc_cfg *cfg) {
    struct mysqlc *self = (struct mysqlc *)malloc(sizeof(*self));

    MYSQL *M = mysql_init(NULL);
    if (M == NULL) {
        free(self);
        return NULL;
    }

    int timeout = 1;
    bool reconn = true;
    mysql_options(M, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(M, MYSQL_OPT_RECONNECT, (const char*)&reconn);
    mysql_options(M, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&timeout);

    if (mysql_real_connect(M, cfg->host, cfg->user, cfg->passwd, cfg->db, cfg->port, NULL, 0) == NULL) {
        sh_error("Mysql connect error: %s", mysql_error(M));
        return NULL;
    }
    sh_info("Mysql connect (%s:%u:%s) ok", cfg->host, cfg->port, cfg->db);
    self->M = M;
    return self;
}

void
mysqlc_free(struct mysqlc *self) {
    if (self->M) {
        mysql_close(self->M);
        self->M = NULL;
    }
    free(self);
}

void
mysqlc_ping(struct mysqlc *self) {
    int r = mysql_ping(self->M);
    sh_info("Mysql ping ... (%d)", r);
}

uint64_t 
mysqlc_select(struct mysqlc *self, const char *sql, int len, 
              struct mysqlc_field_desc *desc, int ndesc, 
        void *data, int rowsz, int maxrow) {
    if (mysql_real_query(self->M, sql, len) != 0) {
        MYSQL_ERROR(self, sql);
        return MYSQLC_INVALID_ID;
    }
    MYSQL_RES* result = mysql_store_result(self->M);
    if (result == NULL) {
        MYSQL_ERROR(self, sql);
        return MYSQLC_INVALID_ID;
    }
    uint64_t row_count = mysql_num_rows(result);
    if(row_count == 0) {
        mysql_free_result(result);
        return row_count;
    }
    struct mysqlc_field_desc *d;
    char *dstrow = (char *)data;
    char *field;
    int i, n = 0;
    MYSQL_ROW row;
    for (row = mysql_fetch_row(result);
        row && n < maxrow; 
        row = mysql_fetch_row(result)) { 
        unsigned long *lens = mysql_fetch_lengths(result);
        for (i=0; i<ndesc; ++i) {
            d = &desc[i];
            field = dstrow + d->offset;
            switch (d->type) {
            case MFIELD_INT:
                *(int32_t*)field = row[i] ? strtol(row[i], NULL, 10) :0;
                break;
            case MFIELD_UINT:
                *(uint32_t*)field = row[i] ? strtoul(row[i], NULL, 10) :0;
                break;
            case MFIELD_INT64:
                *(int64_t*)field = row[i] ? strtoll(row[i], NULL, 10) :0;
                break;
            case MFIELD_UINT64:
                *(uint64_t*)field = row[i] ? strtoull(row[i], NULL, 10) :0;
                break;
            case MFIELD_STRING:
                if (row[i]) {
                    strncpy(field, row[i], d->size);
                    field[d->size-1] = '\0';
                } else {
                    field[0] = '\0';
                }
                break;
            case MFIELD_BIN:
                if (row[i]) {
                    if ((int)lens[i] < d->size) {
                        memcpy(field, row[i], lens[i]);
                        memset(field+lens[i], 0, d->size-lens[i]);
                    } else {
                        memcpy(field, row[i], d->size);
                    }
                } else {
                    memset(field, 0, d->size);
                }
                break;
            case MFIELD_BIN2:
                assert(d->size > 4);
                if (row[i]) {
                    if ((int)lens[i] < d->size-4) {
                        *(uint32_t*)field = lens[i];
                        memcpy(field+4, row[i], lens[i]);
                    } else {
                        *(uint32_t*)field = d->size-4;
                        memcpy(field+4, row[i], d->size-4);
                    }
                } else {
                    *(uint32_t*)field = 0;
                }
                break;
            case MFIELD_INT8:
                *(int8_t*)field = row[i] ? strtol(row[i], NULL, 10) :0;
                break;
            case MFIELD_UINT8:
                *(uint8_t*)field = row[i] ? strtoul(row[i], NULL, 10) :0;
                break;
            case MFIELD_INT16:
                *(int16_t*)field = row[i] ? strtol(row[i], NULL, 10) :0;
                break;
            case MFIELD_UINT16:
                *(uint16_t*)field = row[i] ? strtoul(row[i], NULL, 10) :0;
                break;
            }
        }
        dstrow = dstrow + rowsz;
        ++n;
    }
    mysql_free_result(result);
    return n;
}

uint64_t
mysqlc_execute(struct mysqlc *self, const char *sql, int len) {
    if (mysql_real_query(self->M, sql, len) != 0) {
        MYSQL_ERROR(self, sql);
        return MYSQLC_INVALID_ID;
    }
    return mysql_affected_rows(self->M);
}

uint64_t 
mysqlc_insert_id(struct mysqlc *self) {
    return mysql_insert_id(self->M);
}

uint32_t 
mysqlc_escape_string(struct mysqlc *self, char *to, const char *from, uint32_t len) {
    return mysql_real_escape_string(self->M, to, from, len);
}
