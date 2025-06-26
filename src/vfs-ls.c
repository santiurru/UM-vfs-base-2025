#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s imagen\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    struct inode root_inode;
    if (read_inode(image_path, 1, &root_inode) != 0) {
        fprintf(stderr, "No se pudo leer el directorio raíz\n");
        return EXIT_FAILURE;
    }

    uint8_t buffer[BLOCK_SIZE];
    int block_nbr = get_block_number_at(image_path, &root_inode, 0);
    if (read_block(image_path, block_nbr, buffer) != 0) {
        fprintf(stderr, "No se pudo leer el bloque de directorio\n");
        return EXIT_FAILURE;
    }

    struct dir_entry *entries = (struct dir_entry *)buffer;
    size_t count = BLOCK_SIZE / sizeof(struct dir_entry);

    for (size_t i = 0; i < count; i++) {
        if (entries[i].inode == 0) continue;

        struct inode in;
        if (read_inode(image_path, entries[i].inode, &in) == 0) {
            print_inode(&in, entries[i].inode, entries[i].name);
        }
    }

    return EXIT_SUCCESS;
}