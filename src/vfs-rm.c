#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <imagen> <archivo1> [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        // 1) Buscar el inode del archivo
        int inode_number = dir_lookup(image_path, filename);
        if (inode_number < 0) {
            fprintf(stderr, "vfs-rm: no encontrado: %s\n", filename);
            continue;
        }
        if (inode_number == 0) {
            fprintf(stderr, "vfs-rm: entrada inválida: %s\n", filename);
            continue;
        }

        // 2) Leer el inode y verificar que sea archivo regular
        struct inode in;
        if (read_inode(image_path, inode_number, &in) != 0) {
            fprintf(stderr, "vfs-rm: error al leer inode %d (%s)\n", inode_number, filename);
            continue;
        }
        if (str_file_type(in.mode)[0] != '-') {
            fprintf(stderr, "vfs-rm: no es un archivo regular: %s\n", filename);
            continue;
        }

        // 3) Truncar todos los datos del archivo (libera bloques)
        if (inode_trunc_data(image_path, &in) != 0) {
            fprintf(stderr, "vfs-rm: error truncando datos de %s\n", filename);
            // aun así seguimos intentando liberar el inode
        }

        // 4) Liberar el inode
        if (free_inode(image_path, inode_number) != 0) {
            fprintf(stderr, "vfs-rm: error liberando inode %d (%s)\n", inode_number, filename);
            // seguimos intentando quitar la entrada de directorio
        }

        // 5) Quitar la entrada del directorio raíz
        if (remove_dir_entry(image_path, filename) != 0) {
            fprintf(stderr, "vfs-rm: error eliminando entrada del dir para %s\n", filename);
            continue;
        }

        // Éxito
        printf("Eliminado: %s\n", filename);
    }

    return EXIT_SUCCESS;
}