#ifndef YAHOO_CONNECTIONS_H
#define YAHOO_CONNECTIONS_H

#include "yahoo2_types.h"

void add_to_list(struct yahoo_data *yd, int fd);
void del_from_list(struct yahoo_data *yd);
void del_from_list_by_fd(int fd);

struct yahoo_data * find_conn_by_id(guint32 id);
struct yahoo_data * find_conn_by_fd(int fd);

#endif
