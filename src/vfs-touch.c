// src/vfs-touch.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s imagen archivo1 [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image = argv[1];

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        if (!name_is_valid(filename)) {
            fprintf(stderr, "Nombre inválido: %s\n", filename);
            continue;
        }

        if (dir_lookup(image, filename) != -1) {
            fprintf(stderr, "El archivo ya existe: %s\n", filename);
            continue;
        }

        int inodo_nro = create_empty_file_in_free_inode(image, 0644);
        if (inodo_nro < 0) {
            fprintf(stderr, "No se pudo crear inodo para %s\n", filename);
            continue;
        }

        if (add_dir_entry(image, filename, inodo_nro) != 0) {
            fprintf(stderr, "Error al agregar entrada para %s\n", filename);
            continue;
        }
    }

    return EXIT_SUCCESS;
}
//test comment