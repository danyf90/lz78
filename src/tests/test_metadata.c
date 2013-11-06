/**
 * @file	test_metadata.c
 * @author	Fabio Carrara, Daniele Formichelli
 * @date	May 14, 2013
 * @brief	Test file for metadata module.
 * @internal
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "metadata.h"

int main (int argc, char *argv[]) {

	const char		*const_str = "ThisStringIsUsedToTestWriteOfMetadata";
	char			*str;
	uint8_t			size, type;
	struct  bitio	*bd;

	bd = bopen("metadata_test.dat", 'w');
	if (bd == NULL)
		goto error;

	meta_write(bd, 1, (void*)const_str, strlen(const_str) + 1);
	meta_write(bd, 2, (void*)const_str, 2);
	meta_finalize(bd);

	bclose(bd);

	bd = bopen("metadata_test.dat", 'r');
	if (bd == NULL)
		goto error;

	str = meta_read(bd, &type, &size);
	if (strcmp(str, const_str) != 0 || type != 1 || size != strlen(str) + 1)
		goto error;

	str = meta_read(bd, &type, &size);
	if (str[0] != const_str[0] || str[1] != const_str[1] || type != 2 || size != 2)
		goto error;

	bclose(bd);
	unlink("metadata_test.dat");
	exit(EXIT_SUCCESS);
error:
	bclose(bd);
	unlink("metadata_test.dat");
	exit(EXIT_FAILURE);
}
