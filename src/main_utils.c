/**
 * @file	main_utils.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 17, 2013
 * @brief	Implementation file for utility functions for main file.
 * @internal
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "dictionary.h"
#include "main_utils.h"
#include "verbose.h"

struct timeval time_diff(struct timeval t2, struct timeval t1) {
	t2.tv_sec -= t1.tv_sec;
	t2.tv_usec -= t1.tv_usec;
	if (t2.tv_usec < 0) {
		t2.tv_sec -= 1;
		t2.tv_usec += 1000000;
	}

	return t2;
}

char* print_time(struct timeval t) {

	char *str;
	int h, m;

	str = malloc(16);
	if (str == NULL)
		return NULL;

	h = t.tv_sec / 3600;
	m = (t.tv_sec - h*3600) / 60;
	t.tv_sec = t.tv_sec % 60;

	if (h > 0)
		sprintf(str, "%02dh%02dm%02ds%03dms", h, m, (int)t.tv_sec, (int)t.tv_usec/1000);
	else if (m > 0)
		sprintf(str, "%02dm%02ds%03dms", m, (int)t.tv_sec, (int)t.tv_usec/1000);
	else if (t.tv_sec > 0)
		sprintf(str, "%02ds%03dms", (int)t.tv_sec, (int)t.tv_usec/1000);
	else if (t.tv_usec > 10000) //if time < 10 ms we print also us
		sprintf(str, "%dms", (int)t.tv_usec/1000);
	else
		sprintf(str, "%dms%dus", (int)t.tv_usec/1000, (int)t.tv_usec%1000);

	return str;
}

int check_args(const char* name, int flags, const char* in_file, const char* out_file, uint32_t dict_size, uint32_t ht_size) {
	
	if (in_file != NULL && out_file != NULL && strcmp(in_file, out_file) == 0) {
		fprintf(stderr, "%s: You cannot specify the same argument for -i and -o option\n", name);
		fprintf(stderr, "Try `%s -h' for more information\n", name);
		return -1;
	}
	
	if ((flags & DECOMPRESS_FLAG) && (flags & COMPRESS_FLAG)) {// compression and decompression setted together
		fprintf(stderr, "%s: You cannot specify both -c and -d option\n", name);
		fprintf(stderr, "Try `%s -h' for more information\n", name);
		return -1;
	}
	
	if (!(flags & (COMPRESS_FLAG | DECOMPRESS_FLAG))) { // neither compression or decompression setted
		fprintf(stderr, "%s: You have to specify either -c or -d option\n", name);
		fprintf(stderr, "Try `%s -h' for more information\n", name);
		return -1;
	}
	
	if ((flags & DECOMPRESS_FLAG) && (flags & DICT_SIZE_FLAG)) { // decompression and dict_size setted together
		fprintf(stderr, "%s: You cannot specify both -d and -s option\n", name);
		fprintf(stderr, "Try `%s -h' for more information\n", name);
		return -1;
	}
	
	if ((flags & DECOMPRESS_FLAG) && (flags & TABLE_SIZE_FLAG)) { // decompression and ht_size setted together
		fprintf(stderr, "%s: You cannot specify both -d and -t option\n", name);
		fprintf(stderr, "Try `%s -h' for more information\n", name);
		return -1;
	}
	
	if ((flags & DICT_SIZE_FLAG) || (flags & TABLE_SIZE_FLAG)) { // dict size or table size modified
		if (dict_size < DICT_MIN_SIZE || dict_size > DICT_MAX_SIZE) {
			fprintf(stderr, "%s: Invalid argument for dictionary size\n", name);
			fprintf(stderr, "Try `%s -h' for more information\n", name);
			return -1;
		}
		
		if (ht_size < dict_size) {
			fprintf(stderr, "%s: Invalid arguments for hash table size\n", name);
			fprintf(stderr, "Try `%s -h' for more information\n", name);
			return -1;
		}
	}
	
	return 0;
}

void print_infos(int flags, const char *in_file, const char *out_file, uint32_t dict_size, uint32_t ht_size) {
	
	if (VERBOSE_LEVEL < 1)
		return;
	
	PRINT(1, "\nMode:\t\t\t%s\n", flags & COMPRESS_FLAG ? "Compress" : "Decompress");
	
	PRINT(1, "Input:\t\t\t%s\n", in_file != NULL ? in_file : "Standard Input");
	
	if (out_file != NULL) {
		PRINT(1, "Output:\t\t\t%s\n", out_file);
	}
	else if ((flags & ORIG_FILENAME_FLAG) == 0) {
		PRINT(1, "Output:\t\t\tStandard Output\n");
	}
	
	if (flags & COMPRESS_FLAG) {
		PRINT(1, "Dictionary Size:\t%d\n", dict_size);
		
		PRINT(1, "Hash Table Size:\t%d\n", ht_size);	
	}
	
	PRINT(1, "\n%s Started\n", flags & COMPRESS_FLAG ? "Compression" : "Decompression");
}

void print_stats(int flags, const char *in_file, const char *out_file, uint64_t filesize, struct timeval t1) {
	
	char 			*timestamp;
	struct timeval	t2;
	int				fd;
	
	if (VERBOSE_LEVEL < 1)
		return;
		
	gettimeofday(&t2, NULL);
	t2 = time_diff(t2, t1);
	timestamp = print_time(t2);
	
	if (timestamp != NULL) {
		
		PRINT(1, "%s Time:\t%s\n", flags & COMPRESS_FLAG ? "Compression" : "Decompression", timestamp);
		
		PRINT(1, "Throughput:\t\t%.3f MB/s\n", (((double)filesize) / (((double)t2.tv_sec) + ((double)t2.tv_usec)/1000000)) / (1024*1024));
		
		if ((flags & COMPRESS_FLAG) && out_file != NULL) { // if outfile is stdout i cannot compute compression ratio
			
			fd = open(out_file, O_RDONLY);
			if (fd >= 0) {
				struct stat file_stat;
				fstat(fd, &file_stat);
				PRINT(1, "Original Size:\t\t%.3f MB\n", (double)filesize/(1024*1024));
				PRINT(1, "Compressed Size:\t%.3f MB\n", (double)file_stat.st_size/(1024*1024));
				PRINT(1, "Compression Ratio:\t%.3f\n\n", (double)filesize/file_stat.st_size);
				close(fd);
			}
			else
				PRINT(1, "Error while opening %s to compute compression ratio\n", out_file);
		}
		free(timestamp);
	}
}