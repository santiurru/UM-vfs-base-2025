#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "vfs.h"

#define MAX_FILES 128

// Estructura auxiliar para ordenar
typedef struct {
    char name[FILENAME_MAX_LEN];
    uint32_t inode_number;
} dir_entry_info;

// definimos comparador alfabético
int compare_by_name(const void *a, const void *b) {
    const dir_entry_info *entry1 = a;
    const dir_entry_info *entry2 = b;
    return strcmp(entry1->name, entry2->name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s imagen\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];
    struct inode root_inode;

    // Leer el nodo-i del directorio raíz (inode 1)
    if (read_inode(image_path, 1, &root_inode) != 0) {
        fprintf(stderr, "Error al leer el directorio raíz.\n");
        return EXIT_FAILURE;
    }

    // Leer contenido del directorio
    dir_entry_info entries[MAX_FILES];
    uint32_t total = 0;
    struct dir_entry entry;

    for (uint16_t i = 0; i < root_inode.size / sizeof(struct dir_entry); i++) {
        if (inode_read_data(image_path, 1, &entry, sizeof(struct dir_entry), i * sizeof(struct dir_entry)) != 0)
            continue;

        if (entry.inode == 0) continue; // Entrada vacía

        strncpy(entries[total].name, entry.name, FILENAME_MAX_LEN);
        entries[total].inode_number = entry.inode;
        total++;
    }

    // Ordenar alfabéticamente
    qsort(entries, total, sizeof(dir_entry_info), compare_by_name);

    // Imprimir los inodos ordenados
    for (uint32_t i = 0; i < total; i++) {
        struct inode in;
        if (read_inode(image_path, entries[i].inode_number, &in) != 0) {
            fprintf(stderr, "Error al leer inode %u (%s)\n", entries[i].inode_number, entries[i].name);
            continue;
        }
        print_inode(&in, entries[i].inode_number, entries[i].name);
    }

    return EXIT_SUCCESS;
}