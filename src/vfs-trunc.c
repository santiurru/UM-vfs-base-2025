#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s imagen archivo1 [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    int exit_code = EXIT_SUCCESS;

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        // Buscar inode en directorio raíz
        int inode_num = dir_lookup(image_path, filename);
        if (inode_num <= 0) {
            fprintf(stderr, "Error: '%s' no existe en la imagen.\n", filename);
            exit_code = EXIT_FAILURE;
            continue;
        }

        // Leer inode
        struct inode in;
        if (read_inode(image_path, inode_num, &in) != 0) {
            fprintf(stderr, "Error: no se pudo leer inode de '%s'.\n", filename);
            exit_code = EXIT_FAILURE;
            continue;
        }

        // Verificar que sea archivo regular
        if (str_file_type(in.mode)[0] != '-') {
            fprintf(stderr, "Error: '%s' no es un archivo regular.\n", filename);
            exit_code = EXIT_FAILURE;
            continue;
        }

        // Truncar datos: libera bloques y pone size=0
        if (inode_trunc_data(image_path, &in) != 0) {
            fprintf(stderr, "Error: no se pudo truncar '%s'.\n", filename);
            exit_code = EXIT_FAILURE;
            continue;
        }

        // Escribir inode actualizado
        if (write_inode(image_path, inode_num, &in) != 0) {
            fprintf(stderr, "Error: no se pudo actualizar inode de '%s'.\n", filename);
            exit_code = EXIT_FAILURE;
            continue;
        }

        printf("Archivo truncado: %s (inode %d)\n", filename, inode_num);
    }

    return exit_code;
}
