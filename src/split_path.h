#ifndef SPLIT_PATH_H
#define SPLIT_PATH_H
#include <stdlib.h>

char *parse_filename(char **);
char **split_path(const char *);
void free_split_path(char **, size_t);

#endif
