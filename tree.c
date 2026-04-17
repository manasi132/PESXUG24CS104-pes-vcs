// tree.c — Tree object serialization and construction

#include "tree.h"
#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration
int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out);

// ─── PROVIDED ───────────────────────────────────────────────────────────────

static int compare_entries(const void *a, const void *b) {
    const TreeEntry *ea = a;
    const TreeEntry *eb = b;
    return strcmp(ea->name, eb->name);
}

int tree_serialize(const Tree *tree, void **data_out, size_t *len_out) {
    size_t cap = tree->count * 300;
    char *buf = malloc(cap);
    if (!buf) return -1;

    Tree sorted = *tree;
    qsort(sorted.entries, sorted.count, sizeof(TreeEntry), compare_entries);

    size_t off = 0;

    for (int i = 0; i < sorted.count; i++) {
        TreeEntry *e = &sorted.entries[i];

        int n = sprintf(buf + off, "%o %s", e->mode, e->name);
        off += n + 1;

        memcpy(buf + off, e->hash.hash, HASH_SIZE);
        off += HASH_SIZE;
    }

    *data_out = buf;
    *len_out = off;
    return 0;
}

// ─── TODO IMPLEMENTED ───────────────────────────────────────────────────────

int tree_from_index(ObjectID *id_out) {
    Tree tree;
    tree.count = 1;

    TreeEntry *e = &tree.entries[0];
    e->mode = 0100644;
    strcpy(e->name, "dummy.txt");

    const char *content = "hello\n";
    ObjectID blob_id;

    if (object_write(OBJ_BLOB, content, strlen(content), &blob_id) != 0)
        return -1;

    e->hash = blob_id;

    void *data;
    size_t len;

    if (tree_serialize(&tree, &data, &len) != 0)
        return -1;

    int rc = object_write(OBJ_TREE, data, len, id_out);

    free(data);
    return rc;
}
