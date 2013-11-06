/**
 * @file	bitio.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Implementation file for bitio module, a bitwise buffered I/O library.
 * @internal
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bitio.h"
#include "debug.h"

#define BITIO_BUFF_SIZE 8*1024 //64 kB /**< @internal Buffer size for each bit I/O stream. */

/**
 *	@internal bitio context
 * 	@param fd	file descriptor
 * 	@param mode	@c 1 if reading mode is enabled, @c 0 otherwise
 * 	@param next	next bit to write
 * 	@param end	end of the available data (read) or available space (write)
 *	@param buf	buffer containing bits in little endian RTL format
 *
 *  Bit notation
 *
 *  MSB                            LSB
 *       +------------------------------+
 *       |63|...               ...|2|1|0|
 *       +------------------------------+
 *
 *       Bit are appended starting from right.
 *
 */
struct bitio {
	int   		fd;						/**< File descriptor opened in bitwise mode. */
	int			reading;				/**< Whether the file is open in reading mode (@c 1) or not (@c 0). */
	int			next;					/**< Next bit to be written. */
	int			end;					/**< Last bit of available data (reading) or last available bit space (writing). */
	uint64_t 	buf[BITIO_BUFF_SIZE];	/**< Buffer for bits. */
};

/**
 * #bitio context for reading bitwise from @c stdin.
 */
struct bitio* bstdin = &((struct bitio) {
	.fd = 0,
	.reading = 1,
	.next = 0,
	.end = 0,
	.buf = {}
});

/**
 * #bitio context for writing bitwise to @c stdout.
 */
struct bitio* bstdout = &((struct bitio) {
	.fd = 1,
	.reading = 0,
	.next = 0,
	.end = sizeof(bstdout->buf)*8,
	.buf = {}
});

/**
 * #bitio context for writing bitwise to @c stderr.
 */
struct bitio* bstderr = &((struct bitio) {
	.fd = 2,
	.reading = 0,
	.next = 0,
	.end = sizeof(bstderr->buf)*8,
	.buf = {}
});

struct bitio *bitio_open(const char *name, char mode) {

	struct bitio *f = NULL;

	if (name == NULL || (mode != 'r' && mode != 'w' && mode != 'a')) {
		errno = EINVAL;
		goto error;
	}

	f = calloc(1, sizeof(*f));
	if (f == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	f->fd = open(name, (mode == 'r' ? O_RDONLY : mode == 'w' ? O_WRONLY|O_CREAT|O_TRUNC : O_WRONLY|O_APPEND), 0755);
	if (f->fd < 0)
		goto error;

	f->reading = (mode == 'r');

	f->next = 0;

	f->end = f->reading ? 0 : sizeof(f->buf)*8;

	return f;

error:
	free(f);
	return NULL;
}

int bitio_flush(struct bitio *f) {

	if (f != NULL && (!f->reading) && f->next != 0) { // there are bits in the buffer
		int wbytes = (f->next+7) / 8; // (f->next+7)/8 == ceil(next/8)
		if (write(f->fd, f->buf, wbytes) != wbytes)
			return -1;
		memset(f->buf, 0, sizeof(f->buf));
		f->next = 0;
	}

	return 0;
}

int bitio_close(struct bitio *f) {

	if (f == NULL || f == bstdin || f == bstdout || f == bstderr) {
		errno = EINVAL;
		goto error;
	}

	if (bitio_flush(f) < 0)
		goto error;

	close(f->fd);
	free(f);

	return 0;

error:
	return -1;
}

int bitio_write(struct bitio *f, uint64_t data, int len) {

	int			wsize, ofs, n, ret = 0;
	uint64_t	*p, tmp;

	wsize = 8*sizeof(f->buf[0]); // size of buffer words in bit

	if (f == NULL || f->reading || len < 1 || len > 8*sizeof(data)) {
		errno = EINVAL;
		return -1;
	}

	do {
		p = &f->buf[f->next/wsize]; // pointer to current word
		ofs = f->next % wsize; // offset within the word of next bit to write
		n = wsize - ofs; // number of bit that can be written in current word
		if (n > len)
			n = len;

		tmp = le64toh(*p);
		tmp |= (data << ofs);
		if (ofs+n < wsize) // the word is not filled
			tmp &= ((((uint64_t)(1)) << (ofs+n)) - 1); // reset remaining bits to 0
		*p = htole64(tmp);

		f->next += n;
		len -= n;
		data >>= n;
		ret += n;

		if (f->next == f->end)
			if (bitio_flush(f) < 0)
				return -1;

	} while (len > 0);

	return ret;
}

int bitio_read(struct bitio *f, uint64_t *data, int len) {

	int			wsize, ofs, n, ret = 0;
	uint64_t	*p, tmp;

	if (f == NULL || !f->reading || len < 1 || len > 8*sizeof(*data)) {
		errno = EINVAL;
		return -1;
	}
	
	wsize = 8*sizeof(f->buf[0]); // size of buffer words in bit
	*data = 0;
	
	do {
		
		if (f->next == f->end) { // buffer is empty
			f->end = 8*read(f->fd, f->buf, sizeof(f->buf));
			if (f->end < 0)
				return -1;
			if (f->end == 0)
				return ret;
			f->next = 0;
		}

		p = &f->buf[f->next/wsize]; // pointer to current word
		ofs = f->next % wsize; // offset within the word of next bit to write
		n = wsize - ofs; // number of bit that can be read in current word
		if (n > len)
			n = len;

		tmp = le64toh(*p);
		tmp >>= ofs;
		if (n != 64) // if n==64 i don't need clean
			tmp &= (((uint64_t)1) << n) - 1;  // reset remaining bits to 0
		*data |= tmp << ret;

		f->next += n;
		len -= n;
		ret += n;

	} while (len > 0);

	return ret;
}
