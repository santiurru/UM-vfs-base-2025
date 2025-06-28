#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "vfs.h"

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s imagen archivo1 [archivo2...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    for (int i = 2; i < argc; i++) {
        const char *filename = argv[i];
        
        int inode_number = dir_lookup(image_path, filename);
        if (inode_number <= 0) {
            fprintf(stderr, "Error: el archivo '%s' no existe en la imagen.\n", filename);
            continue;
        }

        struct inode in;
        if (read_inode(image_path, inode_number, &in) != 0) {
            fprintf(stderr, "Error: no se pudo leer el inodo de '%s'.\n", filename);
            continue;
        }

        // Validar que sea un archivo regular (no directorio)
        if (str_file_type(in.mode)[0] != '-') {
            fprintf(stderr, "Error: '%s' no es un archivo regular.\n", filename);
            continue;
        }

        uint8_t buffer[BUFFER_SIZE];
        size_t total_read = 0;

        while (total_read < in.size) {
            size_t bytes_to_read = BUFFER_SIZE;
            if (total_read + bytes_to_read > in.size) {
                bytes_to_read = in.size - total_read;
            }

            if (inode_read_data(image_path, inode_number, buffer, bytes_to_read, total_read) < 0) {
                fprintf(stderr, "Error: lectura fallida en '%s'.\n", filename);
                break;
            }

            fwrite(buffer, 1, bytes_to_read, stdout);
            total_read += bytes_to_read;
        }
    }

    return EXIT_SUCCESS;
}
