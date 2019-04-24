#ifndef NEIGHBOR_H
#define NEIGHBOR_H

static void broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from);

static const struct broadcast_callbacks broadcast_call = {broadcast_recv};

void print_neighbors();


#endif
