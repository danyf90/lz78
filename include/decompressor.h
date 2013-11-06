/**
 * @file	decompressor.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Header file for decompressor interface.
 */

#ifndef __DECOMPRESSOR_H__
#define __DECOMPRESSOR_H__

#include <stdint.h>

#define DEC_ORIG_FILENAME	1	/**< Flag for saving decompressed file with original filename*/

/**
 * Decompress file with name @p in_filename using a dictionary of size @p dict_size and store
 * the output stream in @p fout.
 * If @p in_filename is @c NULL, compressed data is read from @c stdin.
 * The @p flags argument is formed by OR'ing one or more of the following values:
 *
 * @c DEC_ORIG_FILENAME Save decompressed file using original file name.
 * 						Works only if @p out_filename is not @c NULL.
 *
 *	@param	in_filename	Input filename where to read data to be decompressed;
 *						if @c NULL, this function reads data from @c stdin.
 *	@param	fout		Output stream passed as @c FILE* pointer.
 *	@param	flags		Decompression options.
 * .
 *
 *	@return	The size of the output file on success, @c -1 on failure.
 */
int64_t decompress(const char* in_filename, const char* out_filename, uint8_t flags);

#endif
