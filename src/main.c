/**
 * @file	main.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 13, 2013
 * @brief	Main file of lz78 data compressor.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "compressor.h"
#include "decompressor.h"
#include "dictionary.h"
#include "main_utils.h"
#include "verbose.h"

#define DEFAULT_DICT_SIZE	1048576
#define DEFAULT_HT_SIZE		1499933 + NUM_SYMBOLS + 1

const char *help = "\
Usage: lz78 [-c [-s <dict_size] [-t <table_size>] | -d] [-i <input_file>] [-o <output_file>] [-v]\n\n\
\
  -c               compress, cannot be specified together with -d\n\
  -d               decompress, cannot be specified together with -c\n\
  -h               print this help\n\
  -i <input>       input from file instead of stdin\n\
  -m               perform md5 check (only for compression)\n\
  -o [<output>]    output to file instead of stdout, without agruments default filename is <input>.lz78 (compression) or orginal filename (decompression)\n\
  -s <dict_size>   set dictionary size (only for compression), <dict_size> must be between %d and %d\n\
  -t <table_size>  set hash table size (only for compression), <table_size> must be greater than <dict_size>\n\
  -v               be verbose to stdout if -o is specified, otherwise to stderr\n\n";

int main (int argc, char *argv[]) {
	int				c, free_name = 0;
	uint8_t			flags = 0, dec_flags = 0, meta_flags = 0;
	uint32_t		dict_size, ht_size;
	int64_t			filesize;
	char			*in_file = NULL, *out_file = NULL;
	struct timeval	t1;

 	meta_flags = META_DICT_SIZE | META_NAME | META_TIMESTAMP;
	dict_size = DEFAULT_DICT_SIZE;
	ht_size = DEFAULT_HT_SIZE;
	VERBOSE_STREAM = stderr;

	opterr = 0; // don't print error message
	while ((c = getopt(argc, argv, "cdhvi:mo:s:t:")) != -1) {
		switch (c) {
			case 'c':
				flags |= COMPRESS_FLAG;
				break;

			case 'd':
				flags |= DECOMPRESS_FLAG;
				break;

			case 'h':
				printf(help, DICT_MIN_SIZE, DICT_MAX_SIZE);
				exit(EXIT_SUCCESS);

			case 'i':
				in_file = optarg;
				break;

			case 'm':
				meta_flags |= META_MD5;
				break;

			case 'o':
				out_file = optarg;
				VERBOSE_STREAM = stdout;
				break;

			case 's':
				dict_size = atoll(optarg);
				flags |= DICT_SIZE_FLAG;
				break;

			case 't':
				ht_size = atoll(optarg);
				flags |= TABLE_SIZE_FLAG;
				break;
				
			case 'v':
				VERBOSE_LEVEL++;
				break;
				
			case '?': // unknown option or option without required argument
				if (optopt == 'o')
					flags |= ORIG_FILENAME_FLAG;
					break;
				
				if (optopt == 'i' || optopt == 's' || optopt == 't')
					fprintf(stderr, "%s: You cannot specify -%c option without an argument\n", argv[0], optopt);
				else if (isprint (optopt))
					fprintf(stderr, "%s: Unknown option '%c'\n", argv[0], optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
				fprintf(stderr, "Try `%s -h' for more information\n", argv[0]);
				
				exit(EXIT_FAILURE);
		}
	}

	if (check_args(argv[0], flags, in_file, out_file, dict_size, ht_size) < 0) // check if options are valid
		exit(EXIT_FAILURE);

	if (out_file == NULL && (flags & ORIG_FILENAME_FLAG)) { // option -o without argument 
		if (flags & COMPRESS_FLAG) { // compression: out_file will be stdin.lz78 or filename.lz78
			if (in_file == NULL)
				out_file = "stdin.lz78";
			else {
				c = path_len(in_file);
				out_file = malloc(strlen(in_file + c) + 6);
				strcpy(out_file, in_file + c);
				strcat(out_file, ".lz78");
				free_name = 1; // set flag to free(out_file);
			}
		}
		else // decompression: out_file will be original filename if available, stdout otherwise
			dec_flags |= DEC_ORIG_FILENAME;
	}
	
	print_infos(flags, in_file, out_file, dict_size, ht_size);
	gettimeofday(&t1, NULL);
	
	if (flags & COMPRESS_FLAG) 
		filesize = compress(in_file, out_file, dict_size, ht_size, meta_flags);
	else
		filesize = decompress(in_file, out_file, dec_flags);
	
	if (filesize < 0) {
		perror(flags & COMPRESS_FLAG ? "Compression Failed" : "Decompression Failed");
		goto error;
	}
	
	print_stats(flags, in_file, out_file, filesize, t1);

	if (free_name == 1)
		free(out_file);
	exit(EXIT_SUCCESS);
	
error:
	if (free_name == 1)
		free(out_file);
	exit(EXIT_FAILURE);
}