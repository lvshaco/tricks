#ifndef __mysqlc_h__
#define __mysqlc_h__

#include <stdint.h>

#define MYSQLC_INVALID_ID ((uint64_t)-1)

struct mysqlc_cfg {
    const char *host;
    int  port;
    const char *user; 
    const char *passwd;
    const char *db;
};

struct mysqlc_field_desc {
    int type;
    int size;
    int offset;
};

#define MFIELD_INT      0
#define MFIELD_UINT     1
#define MFIELD_INT64    2
#define MFIELD_UINT64   3
#define MFIELD_INT8     4 
#define MFIELD_UINT8    5
#define MFIELD_INT16    6
#define MFIELD_UINT16   7
#define MFIELD_STRING   8
#define MFIELD_BIN      9
#define MFIELD_BIN2     10

struct mysqlc *mysqlc_create(const struct mysqlc_cfg *cfg);
void mysqlc_free(struct mysqlc *self); 
void mysqlc_ping(struct mysqlc *self);
uint64_t mysqlc_select(struct mysqlc *self, const char *sql, int len, 
        struct mysqlc_field_desc *desc, int ndesc, 
        void *data, int rowsz, int maxrow);
// -1 for error, or affected rows
// if `insert ... on duplicate key update`, 1 for insert, 2 for update
uint64_t mysqlc_execute(struct mysqlc *self, const char *sql, int len);
uint64_t mysqlc_insert_id(struct mysqlc *self);
uint32_t mysqlc_escape_string(struct mysqlc *self, char *to, const char *from, uint32_t len);

#endif
