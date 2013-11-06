/**
 * @file	metadata.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Header file for writing metadata before compressed data.
 */

#ifndef __METADATA_H__
#define __METADATA_H__

#include "bitio.h"

/**
 * Finalizes metadata block.
 * It writes a metadata of type @c META_END.
 *	@param	bd	#bitio context.
 *
 *	@return number of writen byte on success, @c -1 on failure.
 */
int meta_finalize(struct bitio *bd);

/**
 * Reads a metadata and returns a pointer to it.
 * If @p type on return contains @c META_END means that there are no more metadata.
 * If @p type on return contains @c META_ERROR means that an error occurred.
 *
 *	@param	bd		#bitio context.
 *	@param	type	Pointer in which store metadata type.
 *	@param	size	Pointer in which store metadata size (in byte).
 *
 *	@return	Pointer to the area that contains metadata content.
 */
void* meta_read(struct bitio *bd, uint8_t* type, uint8_t *size);

/**
 * Writes a metadata.
 *	@param	bd		#bitio context.
 *	@param	type	Metadata type.
 *	@param	data	Pointer to metadata content.
 *	@param	size	Metadata size in byte.
 *
 *	@return number of writen byte on success, @c -1 on failure.
 */
int meta_write(struct bitio *bd, uint8_t type, const void* data, uint8_t size);

#endif
