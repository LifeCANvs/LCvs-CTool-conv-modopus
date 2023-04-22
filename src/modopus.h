#ifndef MODCONV_H
#define MODCONV_H
#include <stdbool.h>
#include <opusenc.h>

#include <libopenmpt/libopenmpt.h>
// Settings for encoding etc.
typedef struct{
  int32_t framesize;
  int32_t samplerate;
  size_t buffersize;
  int32_t repeat_count;
  int32_t interpolation;
  int32_t gain;
  int channels;
  char *filename; 
  char *artist;
  char *title;
  char *date;
  bool auto_comment;
  bool print_sub;
  bool print_meta;
  bool dry_run;
  bool quiet;
}modopus_settings;

typedef union{
  struct{
    char *artist;
    char *title;
    char *date;
    char *message;
    char *type;
  };
  char *strarray[5];
}modopus_comments;

void init_settings(modopus_settings *);
void calc_buffer(modopus_settings *);
void init_comments(modopus_comments *);
void clean_comments(modopus_comments *);
void print_settings(const char *, const char *, const modopus_settings);

bool validate_file(char **);
void supported(void);
openmpt_module *create_mod(const char *, const modopus_settings);
void module_print_metadata(openmpt_module *);
void module_print_subsongs(openmpt_module *);
void module_get_comments(openmpt_module *, modopus_comments *);

OggOpusComments *create_opus_comments(const modopus_comments);
OggOpusEnc *create_opus_encoder(const char *, const modopus_settings, OggOpusComments *comm);

int convert_stream(openmpt_module *, OggOpusEnc *, const modopus_settings);
#endif
