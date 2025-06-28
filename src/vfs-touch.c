#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "vfs.h"


int main(int argc, char *argv[]) {
    //Validamos minimo de argumentos
    if (argc < 3) {
        fprintf(stderr, "Uso: %s imagen archivo1 [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];

        // Validar nombre
        if (!name_is_valid(filename)) {
            fprintf(stderr, "Error: el nombre '%s' no es válido.\n", filename);
            continue;
        }

        // Verificar si ya existe
        int found_inode = dir_lookup(image_path, filename);
        if (found_inode > 0) {
            fprintf(stderr, "Error: el archivo '%s' ya existe en la imagen.\n", filename);
            continue;
        } else if (found_inode < 0) {
            fprintf(stderr, "Error: no se pudo verificar la existencia de '%s'.\n", filename);
            continue;
        }

        // Crear archivo vacío en un inodo libre
        int inode_number = create_empty_file_in_free_inode(image_path, DEFAULT_PERM);
        if (inode_number < 0) {
            fprintf(stderr, "Error: no se pudo crear '%s': sin nodos-i libres.\n", filename);
            continue;
        }

        // Agregar al directorio
        if (add_dir_entry(image_path, filename, inode_number) < 0) {
            fprintf(stderr, "Error: no se pudo agregar '%s' al directorio raíz.\n", filename);
            free_inode(image_path, inode_number); //Liberar el inodo del paso anterior si no se puede agregar al directorio
            continue;
        }

        printf("Archivo creado: %s (inode %d)\n", filename, inode_number);
    }

    return EXIT_SUCCESS;
}