/**
 * @file	main_utils.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 17, 2013
 * @brief	Header file for utility functions for main file.
 * @internal
 */

#ifndef __MAIN_UTILS_H__
#define __MAIN_UTILS_H__

#define COMPRESS_FLAG		1
#define DECOMPRESS_FLAG		2
#define DICT_SIZE_FLAG		4
#define TABLE_SIZE_FLAG		8
#define	ORIG_FILENAME_FLAG	16

#include <sys/time.h>

/**
 * Checks if the command line arguments are correct.
 *	@param name			Name of the executable.
 *	@param flags		Flags containing the information about main option.
 *	@param in_file		Name of the input file.
 *	@param out_file		Name of the output file.
 *	@param dict_size	Size of the dictionary.
 *	@param ht_size		Size of the hash table.
 */
int check_args(const char* name, int flags, const char* in_file, const char* out_file, uint32_t dict_size, uint32_t ht_size);

/**
 * Print information about the inputs of the compressor/decompressor.
 *	@param flags		Flags containing the information about main option.
 *	@param in_file		Name of the input file.
 *	@param out_file		Name of the output file.
 *	@param dict_size	Size of the dictionary.
 *	@param ht_size		Size of the hash table.
 */
void print_infos(int flags, const char *in_file, const char *out_file,  uint32_t dict_size, uint32_t ht_size);

/**
 * Print information on the performance of the decompressor.
 *	@param flags	Flags containing the information about main option.
 *	@param in_file	Name of the input file.
 *	@param out_file	Name of the output file.
 *	@param filesize	Size of the decompressed file.
 *	@param t1		Struct that contains the time before the beginning of the compression/decompression.
 */
void print_stats(int flags, const char *in_file, const char *out_file, uint64_t filesize, struct timeval t1);

/**
 * Returns difference between @p t2 and @p t1 timeval structures.
 *
 *	@param t2	most recent timestamp.
 *	@param t1	less recent timestamp.
 *
 *	@return	difference between t2 and t1.
 */
struct timeval time_diff(struct timeval t2, struct timeval t1);

/**
 * Returns @p t2 timveval structure in a readable format.
 * Memory allocated for the returned string must be freed by the caller.
 *	@param	t	Time to be printed.
 *
 *	@return	Pointer to a string that contain the timestamp in a readable format.
 */
char* print_time(struct timeval t);

#endif