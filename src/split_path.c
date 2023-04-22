#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "split_path.h"

char *parse_filename(char **split_path){
  if(split_path[1] == NULL){
    return NULL;
  }
  size_t outlen = strlen(split_path[1]) + strlen(".opus");
  char *out = calloc(outlen + 1,sizeof(char));
  out = strcat(out,split_path[1]);
  out = strcat(out,".opus");
  return out;
}

/* Creates a string array with dir, basename, ext including '.' and '/'
 * param path the path to file being checked
 * return an array of size 3, containing dir, basename, and ext in order. 
 *        if it doesn't exist, NULL is used instead.
 */
char** split_path(const char *path){
  // if given path is longer than what is possible
  size_t len = strlen(path);
  if(len > PATH_MAX){
    fprintf(stderr, "Path too long\n");
    return NULL;
  }
  // find the last instance of '/' and '.' in the path
  int extind = -1, dirind = -1;
  for(size_t i = 0; i < len; i ++){
    if(path[i] == '.'){
      extind = i;
    }
    else if(path[i] == '/'){
      dirind = i;
    }
  }
  // test for invalid cases
  if(dirind > extind){
    extind = -1;
  }
  size_t namelen = 0;
  if(dirind != -1 && (dirind + 1) < (int)len){
    namelen = strlen(&path[dirind + 1]);
    
  }
  else{
    namelen = strlen(path);
  }
  if(namelen > NAME_MAX){
    fprintf(stderr,"Filename too long\n");
    return NULL;
  }
  char **out = calloc(3,sizeof(char *));
  if(out == NULL){
    fprintf(stderr, "Failed allocating memory\n");
    return NULL;
  }
  if(extind == -1){ // no '.'
    out[2] = NULL;
  }
  else{ // from the dot to the end of path
    out[2] = strdup(&path[extind]);
    if(out[2] == NULL){
      fprintf(stderr, "Failed allocating memory\n");
      return NULL;
    }
  }
  if(dirind == -1){ // no '.'
    out[0] = NULL;
  }
  else{ // from the start of path to the last '/' (including)
    out[0] = strndup(&path[0], dirind + 1);
    if(out[0] == NULL){
      fprintf(stderr, "Failed allocating memory\n");
      return NULL;
    }
  }
  if(dirind + 1 >= (int)len ||(dirind + 1) == extind){
    // if path ends with '/' or noting is between '/' and '.'
    out[1] = NULL;
  }
  else{ // from dirind + 1 to extind - 1
    out[1] = strndup(&path[dirind + 1], extind - dirind - 1);
    if(out[1] == NULL){
      fprintf(stderr, "Failed allocating memory\n");
      return NULL;
    }
  }
  return out;
}

/* Frees the string array from split_path
 * param split_path char* array given by split_path
 * param size the size of the array. Default 3
 */
void free_split_path(char **split_path, size_t size){
  for(size_t i = 0; i < size; i ++){
    if(split_path[i] != NULL){
      free(split_path[i]);
    }
  }
  char **tmp = split_path;
  split_path = NULL;
  free(tmp);
}
