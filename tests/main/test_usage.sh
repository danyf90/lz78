#!/bin/bash

EX_FILE="compress_me"
INVAL_FILE="invalid_compressed"
INEX_FILE="non_existent"
NAME_CHOSEN_FILE="ncf"
COMPR_FILE_META="compressed_stuff.lz"
COMPR_FILE_NO_META="compressed_stuff_stream.lz"
SEED_FILE="stuff"

EXE="../../lz78"

echo -n "Cleaning previous stuff..."
rm -f $EX_FILE.lz78 $COMPR_FILE_META $COMPR_FILE_NO_META $NAME_CHOSEN_FILE stdin*
echo "done"

echo -n "Preparing stuff..."
cat $SEED_FILE | $EXE -co $COMPR_FILE_NO_META
$EXE -ci $SEED_FILE -o $COMPR_FILE_META
echo "done"

echo "Let\'s do some testing..."
echo
echo "### COMPRESSOR ###"

echo "STDIN -> STDOUT (w/o opt. meta)"
cat $EX_FILE | $EXE -cv > /dev/null
echo "STDIN -> NAME-CHOSEN FILE (w/o opt. meta)"
cat $EX_FILE | $EXE -cvo $NAME_CHOSEN_FILE
echo "STDIN -> AUTO FILE (w/o opt. meta)"
cat $EX_FILE | $EXE -cvo

echo "FILE -> STDOUT (w/ META_NAME, META_TS)"
$EXE -cvi $EX_FILE > /dev/null
echo "FILE -> NAME-CHOSEN FILE (w/ META_NAME, META_TS)"
$EXE -cvi $EX_FILE -o $NAME_CHOSEN_FILE
echo "FILE -> AUTO FILE (w/ META_NAME, META_TS)"
$EXE -cvi $EX_FILE -o

echo "INEXISTENT -> *"
$EXE -ci $INEX_FILE -o

echo "### DECOMPRESSOR ###"

echo "STDIN (w/ META_NAME, META_TS) -> STDOUT"
cat $COMPR_FILE_META | $EXE -dv > /dev/null
echo "STDIN (w/ META_NAME, META_TS) -> AUTO FILE (w/ ORIG_NAME, ORIG_TS)"
cat $COMPR_FILE_META | $EXE -dv -o
echo "STDIN (w/ META_NAME, META_TS) -> NAME-CHOSEN FILE (w/ ORIG_TS)"
cat $COMPR_FILE_META | $EXE -dv -o $NAME_CHOSEN_FILE

echo "STDIN (w/o opt. meta) -> STDOUT"
cat $COMPR_FILE_NO_META | $EXE -dv > /dev/null
echo "STDIN (w/o opt. meta) -> AUTO FILE (w/o ORIG_NAME, ORIG_TS)"
cat $COMPR_FILE_NO_META | $EXE -dv -o
echo "STDIN (w/o opt. meta) -> NAME-CHOSEN (w/o ORIG_TS)"
cat $COMPR_FILE_NO_META | $EXE -dv -o $NAME_CHOSEN_FILE

echo "FILE (w/ META_NAME, META_TS) -> STDOUT"
$EXE -dvi $COMPR_FILE_META > /dev/null
echo "FILE (w/ META_NAME, META_TS) -> AUTO FILE (w/ ORIG_NAME, ORIG_TS)"
$EXE -dvi $COMPR_FILE_META -o
echo "FILE (w/ META_NAME, META_TS) -> NAME-CHOSEN (w/ ORIG_TS)"
$EXE -dvi $COMPR_FILE_META -o $NAME_CHOSEN_FILE

echo "FILE (w/o opt. meta) -> STDOUT"
$EXE -dvi $COMPR_FILE_NO_META > /dev/null
echo "FILE (w/o opt. meta) -> AUTO FILE (w/o ORIG_NAME, ORIG_TS)"
$EXE -dvi $COMPR_FILE_NO_META -o
echo "FILE (w/o opt. meta) -> NAME-CHOSEN (w/o ORIG_TS)"
$EXE -dvi $COMPR_FILE_NO_META -o $NAME_CHOSEN_FILE

echo "INVALID STDIN -> *"
cat $INVAL_FILE | $EXE -dvo
echo "INVALID FILE -> *"
$EXE -dvi $INVAL_FILE -o
echo "INEXISTENT FILE -> *"
$EXE -dvi $INEX_FILE -o

echo "...done"