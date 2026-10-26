#ifndef STORAGE_H
#define STORAGE_H

#include "graph.h"

/* Save the active social network graph to a persistent text file */
bool storage_save_network(const Graph *g, const char *filename);

/* Load social network graph from a persistent text file */
bool storage_load_network(Graph *g, const char *filename);

#endif /* STORAGE_H */
