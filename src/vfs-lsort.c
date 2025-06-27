#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "vfs.h"

#define MAX_FILES 1024  // suficiente para la mayoría de directorios pequeños

// Estructura auxiliar para ordenar
typedef struct {
    char name[FILENAME_MAX_LEN + 1];
    uint32_t inode_number;
} dir_entry_info;

// Comparador alfabético
static int compare_by_name(const void *a, const void *b) {
    const dir_entry_info *e1 = a;
    const dir_entry_info *e2 = b;
    return strcmp(e1->name, e2->name);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <imagen>\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *image_path = argv[1];

    // 1) Leer el inodo del directorio raíz (siempre inode 1)
    struct inode root_inode;
    if (read_inode(image_path, 1, &root_inode) != 0) {
        fprintf(stderr, "Error: no se pudo leer el inodo raíz\n");
        return EXIT_FAILURE;
    }

    if (root_inode.blocks == 0) {
        // ningún bloque asignado → directorio vacío
        return EXIT_SUCCESS;
    }

    // 2) Recolectar todas las entradas
    dir_entry_info entries[MAX_FILES];
    uint32_t count = 0;

    // Cada bloque puede contener BLOCK_SIZE/sizeof(struct dir_entry) entradas
    uint32_t per_block = BLOCK_SIZE / sizeof(struct dir_entry);
    uint8_t buffer[BLOCK_SIZE];

    for (uint16_t b = 0; b < root_inode.blocks; b++) {
        int blk = get_block_number_at(image_path, &root_inode, b);
        if (blk < 0) {
            fprintf(stderr, "Aviso: no se pudo obtener bloque %u del dir\n", b);
            continue;
        }
        if (read_block(image_path, blk, buffer) != 0) {
            fprintf(stderr, "Aviso: error al leer bloque %d\n", blk);
            continue;
        }
        struct dir_entry *de = (struct dir_entry *)buffer;
        for (uint32_t i = 0; i < per_block && count < MAX_FILES; i++) {
            if (de[i].inode == 0) 
                continue;
            // Copiar nombre asegurando terminación
            strncpy(entries[count].name, de[i].name, FILENAME_MAX_LEN);
            entries[count].name[FILENAME_MAX_LEN] = '\0';
            entries[count].inode_number = de[i].inode;
            count++;
        }
    }

    if (count == 0) {
        // nada que mostrar
        return EXIT_SUCCESS;
    }

    // 3) Ordenar alfabéticamente
    qsort(entries, count, sizeof(dir_entry_info), compare_by_name);

    // 4) Imprimir con print_inode
    for (uint32_t i = 0; i < count; i++) {
        struct inode in;
        if (read_inode(image_path, entries[i].inode_number, &in) == 0) {
            print_inode(&in, entries[i].inode_number, entries[i].name);
        } else {
            fprintf(stderr, "Aviso: no se pudo leer inode %u\n",
                    entries[i].inode_number);
        }
    }

    return EXIT_SUCCESS;
}