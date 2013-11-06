/**
 * @file	test_bitio.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Test file for bitio module, a bitwise buffered I/O library.
 * @internal
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "bitio.h"

int main(int argc, char* argv[]) {
	struct bitio *bd;
	uint64_t d, r;
	int i;

	//TEST 1: writes 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF23456789ABCDEF
	if ((bd = bitio_open("bitio_test.dat", 'w')) == NULL) {
		perror("bopen(w)");
		exit(EXIT_FAILURE);
	}
	d = 0x0123456789ABCDEF;
	bitio_write(bd, d, 64);
	bitio_write(bd, d, 56);
	d = 0x23456789ABCDEF01;
	bitio_write(bd, d, 64);
	bitio_write(bd, d, 64);
	bitio_close(bd);

	if ((bd = bitio_open("bitio_test.dat", 'r')) == NULL) {
		perror("bopen(r)");
		exit(EXIT_FAILURE);
	}

	for (i = 1; i <= 3; i++) {
		bitio_read(bd, &r, 64);
		if (r != 0x0123456789ABCDEF)
			exit(EXIT_FAILURE);
	}

	bitio_read(bd, &r, 56);
	if (r != 0x23456789ABCDEF)
		exit(EXIT_FAILURE);

	bitio_close(bd);
	//delete file
	unlink("bitio_test.dat");

	//TEST 2: stdin and stdout
	while(bitio_read(bstdin, &r, 1) > 0) {
		bitio_write(bstdout, r, 1);
	}

	bitio_flush(bstdout);

	exit(EXIT_SUCCESS);
}