#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

#include <libopenmpt/libopenmpt.h>
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>
#include <opusenc.h>

#include "modopus.h"

// Setup modopus_settings to "default" values
void init_settings(modopus_settings *opt){
  opt->framesize = OPUS_FRAMESIZE_20_MS;
  opt->samplerate = 48000;
  opt->buffersize = 960;
  opt->repeat_count = 0;
  opt->interpolation = 0;
  opt->gain = 0;
  opt->channels = 2;
  opt->filename = ""; 
  opt->artist = NULL;
  opt->title = NULL;
  opt->date = NULL;
  opt->auto_comment = false;
  opt->print_sub = false;
  opt->print_meta = false;
  opt->dry_run = false;
  opt->quiet = false; 
}

void calc_buffer(modopus_settings *opt){
  // Calculates buffersize based on framesize and samplerate.
  // For example, sample rate of 48 kHz and frame size of 20 ms results in a buffersize of 960. 
  switch (opt->framesize){
    case OPUS_FRAMESIZE_2_5_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.0025);
      break;
    case OPUS_FRAMESIZE_5_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.005);
      break;
    case OPUS_FRAMESIZE_10_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.01);
      break;
    case OPUS_FRAMESIZE_20_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.02);
      break;
    case OPUS_FRAMESIZE_40_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.04);
      break;
    case OPUS_FRAMESIZE_60_MS:
      opt->buffersize = (int32_t) (opt->samplerate * 0.06);
      break;
  }
}

void init_comments(modopus_comments *comments){
  comments->artist = "";
  comments->title = "";
  comments->date = "";
  comments->message = "";
  comments->type = "";
}

void clean_comments(modopus_comments *comments){
  for(size_t i = 0; i < 5; i ++){
    if(strcmp(comments->strarray[i],"") != 0){
      free(comments->strarray[i]);
    }
  }
}



void print_settings(const char *inpath, const char *outpath, const modopus_settings opt){
    printf("Input:          %s\n",inpath);
    printf("Output:         %s\n",outpath);
    printf("Channels:       %d\n",opt.channels);
    printf("Sample rate:    %d Hz\n",opt.samplerate);
    printf("Play count:     %d + 1 times\n",opt.repeat_count);
    printf("Gain:           %d mB\n",opt.gain);
    printf("Interpolation:  %d\n",opt.interpolation);
    printf("Auto comments:  %d\n\n",opt.auto_comment);
}

/* Checks if input file can be opened in libopenmpt
 * param split_path file path split into 3 parts from another function
 */
bool validate_file(char **split_path){
  // check if file format is not supported, 
  char *ext = split_path[2];
  if(ext != NULL && openmpt_is_extension_supported(&ext[1])){
      //printf("Valid input\n");
      return true;
  }
  fprintf(stderr,"File type not compatible with libopenmpt\n");
  return false;
}

/* Prints the list of supported file types.
 */
void supported(void){
  const char *supported = openmpt_get_supported_extensions();
  printf("Current supported formats are:\n%s\n",supported);
}


/* Creates an openmpt_module with proper (?) error handling
 * param path path to input file
 * param opt options struct with values set
 */
openmpt_module *create_mod(const char *path, const modopus_settings opt){
  // Open input file and load to openmpt module
  FILE *infile = NULL;
  openmpt_module *mod = NULL;
  
  int error = OPENMPT_ERROR_OK;

  infile = fopen(path, "rb");
  if(infile == NULL){
    fprintf(stderr, "%s: %s\n",path, strerror(errno));
    return NULL;
  }
  mod = openmpt_module_create2(
      openmpt_stream_get_file_callbacks(),
      infile,
      NULL,
      NULL,
      NULL,
      NULL,
      &error,
      NULL,
      NULL
  );
  fclose(infile);
  if(mod == NULL){
    fprintf(stderr, "%s: failed creating openmot_module\n",path);
    return NULL;
  }

  // Set render options for openmpt
  error = openmpt_module_set_repeat_count(mod, opt.repeat_count);
  if(error == 0){
    fprintf(stderr,"%s: failed setting repeat count\n",path);
    openmpt_module_destroy(mod);
    return NULL;
  }
  error = openmpt_module_set_render_param(
      mod,
      OPENMPT_MODULE_RENDER_INTERPOLATIONFILTER_LENGTH,
      opt.interpolation
  );
  if(error == 0){
    fprintf(stderr,"%s: failed setting interpolation param\n",path);
    openmpt_module_destroy(mod);
    return NULL;
  }
  error = openmpt_module_set_render_param(
      mod,
      OPENMPT_MODULE_RENDER_MASTERGAIN_MILLIBEL,
      opt.gain
  );
  if(error == 0){
    fprintf(stderr,"%s: failed setting master gain\n",path);
    openmpt_module_destroy(mod);
    return NULL;
  }
  return mod;
}

void module_print_metadata(openmpt_module *mod){
  printf("Printing metadata:\n");
  const char *tmp = openmpt_module_get_metadata_keys(mod);
  char *keys = strdup(tmp);
  char *key = strtok(keys, ";");
  while(key != NULL){
    const char *data = openmpt_module_get_metadata(mod, key);
    printf("%s:\"%s\"\n",key,data);
    free((char *)data);
    key = strtok(NULL,";");
  }
  free(keys);
  free((char *)tmp);
}

void module_print_subsongs(openmpt_module *mod){
  printf("Printing subsong data:\n");
  int32_t num_subsongs = openmpt_module_get_num_subsongs(mod);
  for(int i = 0; i < num_subsongs; i ++){
    const char *subsong_name = openmpt_module_get_subsong_name(mod,i); 
    printf("%d: %s\n",i,subsong_name);
    free((char *)subsong_name);
  }
}

void module_get_comments(openmpt_module *mod, modopus_comments *comments){
  char *keys[5] = {"artist", "title", "date", "message", "type_long"};
  for(size_t i = 0; i < 5; i ++){
    const char *string = openmpt_module_get_metadata(mod,keys[i]);
    if(strcmp(string, "") != 0){
      comments->strarray[i] = strdup(string);
    }
    free((char *)string);
  }
}

OggOpusComments *create_opus_comments(const modopus_comments comments){
  int error = OPE_OK;
  OggOpusComments *comm = ope_comments_create();
  if(error != OPE_OK){
    fprintf(stderr,"Failed creating opus comments\n");
    return NULL;
  }
  
  // Check each comment type in modopus_comments
  // if a comment exists, write to output file
  char *keys[5] = {"artist", "title", "date", "message", "type_long"};
  for(size_t i = 0; i < 5; i ++){
    if(strcmp(comments.strarray[i], "") != 0){
      error = ope_comments_add(comm, keys[i], comments.strarray[i]);
      if(error != OPE_OK){//opus_error(error, "set date", NULL, comm)){
        fprintf(stderr, "Failed adding comment %s\n",keys[i]);
        ope_comments_destroy(comm);
        return NULL;
      }
    }
  }
  return comm;
}

OggOpusEnc *create_opus_encoder(const char *outpath, const modopus_settings opt, OggOpusComments *comm){
  int error = OPE_OK;
  OggOpusEnc *enc = ope_encoder_create_file(outpath, comm, opt.samplerate, opt.channels, 0, &error);
  if(error != OPE_OK){
    fprintf(stderr, "Failed creating opus encoder\n");
    return NULL;
  }
  return enc;
}

int convert_stream(openmpt_module *mod, OggOpusEnc *enc, const modopus_settings opt){
  // Reads the input file and sends pcm data to encoder, in increments of buffersize. 
  // Buffer stores interleaved pcm data
  float buffer[opt.buffersize * 2];
  int error = OPE_OK;
  while(1){
    size_t count = 0;
    count = openmpt_module_read_interleaved_float_stereo(mod, opt.samplerate, opt.buffersize, buffer);
    if(count == 0)
      break;
    error = ope_encoder_write_float(enc, buffer, count);
    if(error != OPE_OK){
      fprintf(stderr, "Failed writing opus data\n");
      return 1;
    }
  }
  return 0;
}
