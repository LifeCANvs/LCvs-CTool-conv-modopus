#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include <libopenmpt/libopenmpt.h>
#include <opusenc.h>

#include "modopus.h"
#include "split_path.h"

/* Prints information on how to use options.
 * param name the name of the executable
 */
void usage(const char* name){
  printf("Usage:\n");
  printf("  %s <option(s)> <input filename>\n",name);
  printf("The output file will have the same name as the input file, with .opus file extension.\n");
  printf("\nOptions:\n");
  printf("  -h, --help         Shows this.\n");
  printf("  --supported        Shows the list of supported file formats.\n");
  printf("  -o n               Output directory. Must exist before using.\n");
  printf("  -q, --quiet        Runs without printing information.\n");
  printf("\nRendering options:\n");
  printf("  --samplerate n     Set samplerate to n.\n");
  printf("  --framesize n      Set the frame size (ms) to n.\n");
  printf("                     Must be the following [2.5, 5, 10, 20, 40, 60].\n");
  printf("  --repeat-count n   Repeat the song n times after playing once.\n");
  printf("  --interpolation n  Set interpolation filter length to n.\n");
  printf("                     Values currently supported are:\n" );
  printf("                       0: internal default\n");
  printf("                       1: no interpolation (zero order hold)\n");
  printf("                       2: linear interpolation\n");
  printf("                       4: cubic interpolation\n");
  printf("                       8: windowed sinc with 8 taps\n");
  printf("  --gain n           Set master gain in mB to n.\n");
  printf("  --dry-run          Run the program, skipping writing to file.\n");
  printf("\nComment options:\n");
  printf("  --auto-comment     Copies comments from input file.\n");
  printf("                     [artist, title, date, mesage, and the tracker type]\n");
  printf("  --artist n         Set artist to n.\n");
  printf("  --title n          Set title to n.\n");
  printf("  --date n           Set date to n. Format YYYY-MM-DD, YYYY-MM, or YYYY\n");
  printf("\nPrint options:\n");
  printf("  --print-subsongs   Print subsong data.\n");
  printf("  --print-metadata   Print song metadata.\n");
}

int main(int argc, char **argv){
  if(argc < 2){
    printf("Usage: %s <option(s)> <input filename>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  modopus_settings opt;
  init_settings(&opt);
  
  // Process options.
  int c = 0;
  while(1){
    int option_index = 0;
    const char* opname;
    static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"supported", no_argument, 0, 0},
      {"samplerate", required_argument, 0, 0},
      {"framesize", required_argument, 0, 0},
      {"auto-comment", no_argument, 0, 0},
      {"artist", required_argument, 0, 0},
      {"title", required_argument, 0, 0},
      {"date", required_argument, 0, 0},
      {"repeat-count", required_argument, 0, 0},
      {"interpolation", required_argument, 0, 0},
      {"gain", required_argument, 0, 0},
      {"print-subsongs", no_argument, 0, 0},
      {"print-metadata", no_argument, 0, 0},
      {"dry-run", no_argument, 0, 0},
      {"quiet", no_argument, 0, 'q'},
      {0, 0, 0, 0}
    };
    c = getopt_long(argc, argv, "hqo:", long_options, &option_index);
    if( c == -1)
      break;

    switch (c){
      case 0:
        opname = long_options[option_index].name;
        if(strcmp(opname,"samplerate") == 0){ /* set sample rate */
          opt.samplerate = atoi(optarg);
        }
        else if(strcmp(opname,"framesize") == 0){ /* set frame size */
          if(strcmp(optarg,"2.5") == 0) 
            opt.framesize = OPUS_FRAMESIZE_2_5_MS;
          else if(strcmp(optarg,"5") == 0) 
            opt.framesize = OPUS_FRAMESIZE_5_MS;
          else if(strcmp(optarg,"10") == 0) 
            opt.framesize = OPUS_FRAMESIZE_10_MS;
          else if(strcmp(optarg,"20") == 0)
            opt.framesize = OPUS_FRAMESIZE_20_MS;
          else if(strcmp(optarg,"40") == 0)
            opt.framesize = OPUS_FRAMESIZE_40_MS;
          else if(strcmp(optarg,"60") == 0)
            opt.framesize = OPUS_FRAMESIZE_60_MS;
          else{
            printf("--framesize must be one of the following: [2.5, 5, 10, 20, 40, 60].\n");
            return 1;
          }
        }
        else if(strcmp(opname, "auto-comment") == 0){ // add comments from input file
          opt.auto_comment = true;
        }
        else if(strcmp(opname, "artist") == 0){ // set artist tag
          opt.artist = optarg;
        }
        else if(strcmp(opname, "title") == 0){ // set title tag
          opt.title = optarg;
        }
        else if(strcmp(opname, "date") == 0){ // set date tag
          opt.date = optarg;
        }
        else if(strcmp(opname, "repeat-count") == 0){ // plays n + 1 times
          int32_t rc = atoi(optarg);
          if(rc < 0){
            printf("--repeat-count must be 0 (no loops) or n > 0 (play once and loop n times)\n");
            return 1;
          }
          opt.repeat_count = rc;
        }
        else if(strcmp(opname, "gain") == 0){ // set gain in mB
          opt.gain = atoi(optarg);
        }
        else if(strcmp(opname, "interpolation") == 0){ // set interpolation filter preset
          int32_t ifl = atoi(optarg);
          if(ifl < 0){
            printf("Interpolation value must be grater than 0, see --help for more\n");
            return 1;
          }
          opt.interpolation = ifl;
        }
        else if(strcmp(opname, "print-subsongs") == 0){ // print list of subsongs and numbers
          opt.print_sub = true;
        }
        else if(strcmp(opname, "print-metadata") == 0){ // print song metadata
          opt.print_meta = true;
        }
        else if(strcmp(opname, "dry-run") == 0){ // skips encoding to file
          opt.dry_run = true;
        }
        else if(strcmp(opname, "supported") == 0){ // print list of supported files
          supported();
          return 0;
        }
        break;
      case 'o': // set output file location
        opt.filename = optarg;
        break;
      case 'h': // print usage
        usage(argv[0]);
        return 0;
        break;
      case 'q': // don't print
        opt.quiet = true;
        break;
      case '?': // option not recognized
        usage(argv[0]);
        return 1;
        break;
    }
  }
  
  // Calculate required buffer size based on input values
  calc_buffer(&opt);

  // Run for each input file.
  while(optind < argc){
    // Create openmpt module
    char *filepath = argv[optind++];
    char **split = split_path(filepath);
    if(!validate_file(split)){
      continue;
    }
    openmpt_module *mod = create_mod(filepath, opt);
    if(mod == NULL){
      free_split_path(split, 3);
      continue;
    }

    // Print subsong and metadata info
    if(opt.print_meta){
      module_print_metadata(mod);
    }
    if(opt.print_sub){
      module_print_subsongs(mod);
    }
    
    // Moves onto the next file if dry run.
    if(opt.dry_run){
      openmpt_module_destroy(mod);
      free_split_path(split, 3);
      continue;
    }

    // Store comments settings in modopus_comments
    modopus_comments comments;
    init_comments(&comments);
    // Auto comments
    if(opt.auto_comment){
      module_get_comments(mod,&comments);
    }
    // Adds user defined comments
    if(opt.artist != NULL){
      comments.artist = strdup(opt.artist);
    }
    if(opt.title != NULL){
      comments.title = strdup(opt.title);
    }
    if(opt.date != NULL){
      comments.date = strdup(opt.date);
    }

    char *outpath = NULL;
    outpath = parse_filename(split); 
    if(outpath == NULL){
      clean_comments(&comments);
      openmpt_module_destroy(mod);
      free_split_path(split, 3);
      continue;
    }

    OggOpusComments *comm;
    comm = create_opus_comments(comments);
    if(comm == NULL){
      free(outpath);
      clean_comments(&comments);
      openmpt_module_destroy(mod);
      free_split_path(split, 3);
      continue;
    }
    
    OggOpusEnc *enc;
    enc = create_opus_encoder(outpath, opt, comm);
    if(enc == NULL){
      free(outpath);
      clean_comments(&comments);
      openmpt_module_destroy(mod);
      free_split_path(split, 3);
      ope_comments_destroy(comm);
      continue;
    }
  
    if(!opt.quiet){
      print_settings(filepath, outpath, opt);
    }
       
    // Transfer pcm output from openmpt module to opus encoder
    int error = convert_stream(mod, enc, opt);
    if(error != 0){
      free(outpath);
      ope_encoder_destroy(enc);
      ope_comments_destroy(comm);
      openmpt_module_destroy(mod);
      free_split_path(split, 3);
      free(outpath);
      clean_comments(&comments);
      printf("failed\n");
      continue;
    }

    // Cleanup
    ope_encoder_drain(enc);
    ope_encoder_destroy(enc);
    ope_comments_destroy(comm);
    openmpt_module_destroy(mod);
    free_split_path(split, 3);
    free(outpath);
    clean_comments(&comments);
  }
  exit(EXIT_SUCCESS);
}
