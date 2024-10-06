#!/usr/bin/bash

# Solo funciona con GCC 8.4.1 (que sepamos)
#
# Hacer make en mambo, da error pero luego compila con este script

export RUTA_MAMBO="/misc/practicas/Procesadores/mambo"
export RUTA_COMUN="/misc/practicas/Procesadores/alumnos/comun"

rm dbm

/usr/bin/gcc -D_GNU_SOURCE -g -std=gnu99 -O2 -DGIT_VERSION=\"\" -I$(pwd) -DPLUGINS_NEW \
    -ldl -Wl,-Ttext-segment=0x7000000000 -DDBM_LINK_UNCOND_IMM \
	-DDBM_INLINE_UNCOND_IMM -DDBM_LINK_COND_IMM -DDBM_LINK_CBZ \
	-DDBM_LINK_TBZ -DDBM_TB_DIRECT -DLINK_BX_ALT -DDBM_INLINE_HASH \
	-DDBM_TRACES  -I/usr/include/libelf -o dbm $RUTA_MAMBO/dispatcher.S \
	$RUTA_MAMBO/common.c $RUTA_MAMBO/dbm.c $RUTA_MAMBO/traces.c \
	$RUTA_MAMBO/syscalls.c $RUTA_MAMBO/dispatcher.c $RUTA_MAMBO/signals.c \
	$RUTA_MAMBO/util.S $RUTA_MAMBO/api/helpers.c \
	$RUTA_MAMBO/api/plugin_support.c $RUTA_MAMBO/api/branch_decoder_support.c \
	$RUTA_MAMBO/api/load_store.c $RUTA_MAMBO/api/internal.c \
	$RUTA_MAMBO/api/hash_table.c $RUTA_MAMBO/elf/elf_loader.o \
	$RUTA_MAMBO/elf/symbol_parser.o $RUTA_MAMBO/scanner_a64.c \
	$RUTA_MAMBO/api/emit_a64.c $RUTA_MAMBO/plugins/traduce.c \
	$RUTA_MAMBO/plugins/mtrace.S \
	$RUTA_COMUN/predec.c \
	$(pwd)/cpu.c $RUTA_MAMBO/pie/pie-a64-field-decoder.o \
	$RUTA_MAMBO/pie/pie-a64-encoder.o $RUTA_MAMBO/pie/pie-a64-decoder.o \
	-lelf -lpthread -lz
