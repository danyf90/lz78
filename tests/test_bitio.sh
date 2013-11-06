#!/bin/bash

build/test_bitio < tests/test_bitio_in > tests/test_bitio_out
if $(diff tests/test_bitio_in tests/test_bitio_out); then
	rm tests/test_bitio_out;
	exit 0;
fi

exit 1;
