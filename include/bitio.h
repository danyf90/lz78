/**
 * @file	bitio.h
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 13, 2013
 * @brief	Header file for bitio module, a bitwise buffered I/O library.
 */

#ifndef __BITIO_H__
#define	__BITIO_H__

#include <stdint.h>

//bitio context
struct bitio;

extern struct bitio* bstdin;	/**< bitio context for stdin. */
extern struct bitio* bstdout;	/**< bitio context for stdout. */
extern struct bitio* bstderr;	/**< bitio context for stderr. */

/**
 * Opens the file with filename @p name in the bitio mode specified by @p mode.
 *
 * 	@param	name	Name of the file to open.
 * 	@param	mode 	Open mode (read (r), write (w) or append (a)).
 *
 *	@return	Pointer to bitio context.
 */
struct bitio* bitio_open(const char *name, char mode);

/**
 * Writes @p len bits from @p data to @p fd.
 *
 * 	@param	f		Pointer to #bitio context
 * 	@param	data	Data to be written.
 * 	@param	len		Number of bits to be written.
 *
 * 	@return	@p len on succes, @c -1 otherwise
 */
int bitio_write(struct bitio *f, uint64_t data, int len);

/**
 * Reads at most @p len bits from @p fd to @p data.
 *
 * 	@param	f		Pointer to #bitio context
 * 	@param	data	Pointer to destination area.
 * 	@param	len		Number of bits to be read.
 *
 * 	@return	Number of read bits on succes, @c -1 otherwise.
 */
int bitio_read(struct bitio *f, uint64_t *data, int len);

/**
 * Flushes the buffer to the correspondent file descriptor.
 * 	@param	f		Pointer to #bitio context
 *
 *	@return	@c 0 on success, @c -1 otherwise.
 */
int bitio_flush(struct bitio *f);

/**
 * Flushes the buffer and closes a file in bitio mode.
 * This function returns @c -1 in the case an error occurs when trying to flush
 * the buffer or if you're trying to close @c bstdin, @c bstdout or @c bstderr
 * bit streams.
 *
 * 	@param	f	Pointer to #bitio context
 *
 *	@return	0 on success, -1 otherwise.
 */
int bitio_close(struct bitio *f);

#endif
