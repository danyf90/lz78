/**
 * @file	compressor.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 13, 2013
 * @brief	Header file for compressor interface.
 */

#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <stdint.h>

/**
 * Compress file @p in_filename using a dictionary of size @p dict_size and store
 * the output in file @p out_filename.
 * If @p out_filename is @c NULL, compressed data is redirected to @c stdout.
 * The @p flags argument is formed by OR'ing one or more of the following values@:
 *
 * @c META_DICT_SIZE	Store dictionary size used for compression.
 * @c META_MD5			Store md5 sum of input file; if this metadata is inserted,
 *						decompressor can check decompressed file consistency.
 * @c META_TIMESTAMP	Store original file creation timestamp.
 *
 * @see	#metadata
 *
 *	@param	fin				Input stream passed as @c FILE* pointer.
 *	@param	out_filename	Output filename where to store compressed data;
 *							if @c NULL, this function redirects data to @c stdout.
 *	@param	dict_size		Dictionary size in number of records.
 *	@param	ht_size			Hash table size in number of records.
 *	@param	flags			Indicates whether metadata should be written or not.
 *
 *	@return	The size of original file on success,  @c -1 on failure.
 */
int64_t compress(const char* in_filename, const char* out_filename, uint32_t dict_size, uint32_t ht_size, uint8_t flags);

#endif
