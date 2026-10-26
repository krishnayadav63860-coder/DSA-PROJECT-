#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NAME_LEN 64
#define MAX_INFO_LEN 128
#define INITIAL_GRAPH_CAPACITY 16
#define HASH_TABLE_CAPACITY 101

/* Utility helper functions for input and string manipulation */
void trim_whitespace(char *str);
int str_case_cmp(const char *s1, const char *s2);
void safe_read_line(char *buffer, size_t size);
int safe_read_int(int min_val, int max_val);

#endif /* COMMON_H */
