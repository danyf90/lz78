/**
 * @file	common.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 13, 2013
 * @brief	Header file for common utility functions and macros used by more modules.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define NUM_SYMBOLS		256			/**< Number of symbols in the alphabet. */
#define COUNT_THRESHOLD	1024*1024	/**< After how many bytes a progress indicator (dot) is printed. */

#define META_END		0	/**< Type of last marker that indicates the end of metadata stream. */
#define META_DICT_SIZE	1	/**< Metadata field type flag for dictionary size. */
#define META_NAME		2	/**< Metadata field type flag for original filename. */
#define META_TIMESTAMP	4	/**< Metadata field type flag for file creation timestamp. */
#define META_MD5		8	/**< Metadata field type flag for md5 sum. */
#define META_ERROR		255	/**< Error code for meta_ functions. */

/**
 * Computes the message digest of file @p fin, using the algorithm
 * specified by @p md_name, and returns a pointer to the computed digest.
 * The size of the computed digest is returned in @p size.
 * Memory allocated to contain the digest must be freed by caller.
 *
 *	@param	fin		File of which the digest must be calculated, specified as
 					a @c FILE*.
 *	@param	md_name	String containing the name of the algorithm
 *					(e.g. "md5", "sha1", ...). You can specify any algorithm
 *					supported by current version of OpenSSL library.
 *	@param	size	Pointer to @c int where the size of produced digest is put.
 *	@return			Pointer	to computed digest on success, @c NULL on failure.
 *
 * @code
 *	FILE	*fi;
 *	int		digest_size;
 *	void*	md5sum;
 *
 *	fi = fopen("my_file.dat", "r");
 *	md5sum = compute_digest(fi, "md5", &digest_size);
 *	if (md5sum != NULL) {
 *		// use message digest
 *		free(md5sum);
 *	}
 * @endcode
 */
unsigned char* compute_digest(FILE *fin, const char *md_name, int *size);

/**
 * Converts @p size raw bytes pointed by @p buff in hexadecimal readable format
 * and return them as a string. Memory allocated for the converted string must
 * be freed by the caller.
 *
 *	@param	buff	Pointer to raw bytes to be converted.
 *	@param	size	Number of bytes to be converted.
 *	@return			Pointer to a (@p size *2) lenght buffer which contains
 					hexadecimal characters of the converted bytes.
 */
char* sprinth(const unsigned char *buff, int size);

/**
 * Returns the length of the path of @p filename without considering the part after last '/' (file name).
 *
 *	@param	filename	Path to process.
 */
int path_len(const char* filename);

#endif