/**
 * @file	metadata.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for metadata module.
 * @internal
 */

#include <errno.h>
#include <stdlib.h>

#include "common.h"
#include "metadata.h"

#define	min(a,b) ((a) < (b) ? (a) : (b))

int meta_finalize(struct bitio *bd) {

	return meta_write(bd, 0, NULL, 0);
}

void* meta_read(struct bitio *bd, uint8_t *type, uint8_t *size) {

	int			i;
	uint8_t		read_step, to_read;
	uint64_t	tmp = 0;
	void		*data;

	if (bd == NULL || type == NULL || size == NULL) {
		errno = EINVAL;
		goto error;
	}

	// read metadata type
	if (bitio_read(bd, &tmp, 8) != 8)
		goto error;

	*type = (uint8_t)tmp;

	if (*type == 0)
		return NULL;

	// read metadata size
	if (bitio_read(bd, &tmp, 8) != 8)
		goto error;
	*size = (uint8_t)tmp;
	to_read = (*size);

	// allocate are for metadata
	data = malloc(8*((int)(*size + 7)/8));
	if (data == NULL)
		goto error;

	// read metadata: bread allows to read only 64 bits at time
	read_step = 8;
	for (i = 0; to_read > 0; i++) {

		read_step = min(to_read, read_step); // reads at most 8 byte

		if (bitio_read(bd, ((uint64_t*)data) + i, read_step*8) != read_step*8) {
			free(data);
			goto error;
		}

		
		to_read -= read_step;
	}

	return data;

error:
	*type = META_ERROR;
	return NULL;
}

int meta_write(struct bitio *bd, uint8_t type, const void* data, uint8_t size) {

	int			i;
	uint8_t		write_step, orig_size;
	uint64_t	data_chunk;

	if (bd == NULL || (data == NULL && type != 0)) {
		errno = EINVAL;
		return -1;
	}

	// write metadata type
	if (bitio_write(bd, type, 8) != 8)
		return -1;

	if (type == 0)
		return sizeof(type);

	// write metadata size
	if (bitio_write(bd, size, 8) != 8)
		return -1;

	// write metadata: bwrite can write only 64 bits at time
	write_step = 8;
	orig_size = size;
	for (i = 0; size > 0; i++) {
		data_chunk = ((uint64_t*)data)[i];

		write_step = min(size, write_step); // writes at most 8 byte

		if (bitio_write(bd, data_chunk, write_step*8) != write_step*8)
			return -1;

		size -= write_step;
	}

	return sizeof(type) + sizeof(size) + orig_size;
}