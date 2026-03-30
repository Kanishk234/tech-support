#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab4.h"

extern Node *g_root;

#define MAGIC   0x54454348u   /* "TECH" */
#define VERSION 1u
#define FAIL(q, f)     \
    do {               \
        q_free(q);     \
        fclose(f);     \
        return 0;      \
    } while(0)         \

#define FW(ptr, size, count, f, q)                      \
    do {                                                \
        if (fwrite(ptr, size, count, f) != count) {     \
            FAIL(q, f);                                 \
        }                                               \
    } while (0)                                         \

#define FREEFLAT(arr, i)                                \
    do {                                                \
        for (uint32_t j = 0; j < i; j++) {              \
            free_tree(arr[j]);                          \
        }                                               \
    } while (0)                                         \

#define FR_NOARRAY(ptr, size, count, f)                 \
    do {                                                \
        if (fread(ptr, size, count, f) != count) {      \
            fclose(f);                                  \
            return 0;                                   \
        }                                               \
    } while (0)                                         \

#define FR(ptr, size, count, f, arr, i)                 \
    do {                                                \
        if (fread(ptr, size, count, f) != count) {      \
            FREEFLAT(arr, i);                           \
            fclose(f);                                  \
            return 0;                                   \
        }                                               \
    } while (0)                                         \


typedef struct { Node *node; int id; } NodeMapping;

/* ----------------------------------------------------------------
 * TODO 27  save_tree
 *
 * Serialize the entire tree to a binary file using BFS order.
 *
 * File format:
 *   Header:  uint32 magic | uint32 version | uint32 nodeCount
 *   Per node (BFS order):
 *     uint8  isQuestion
 *     uint32 textLen          (bytes, no null terminator in file)
 *     char[] text             (exactly textLen bytes)
 *     int32  yesId            (-1 if NULL)
 *     int32  noId             (-1 if NULL)
 *
 * Return 1 on success, 0 on failure.
 * ---------------------------------------------------------------- */
int save_tree(const char *filename) {
    if (g_root == NULL) { return 0; }

    // check integrity of tree
    int success = check_integrity();
    if (success == 0) { return 0; }

    FILE *f = fopen(filename, "wb");
    if (f == NULL) { return 0; }
    Queue q;
    q_init(&q);

    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t nodeCount = count_nodes(g_root);
    FW(&magic, sizeof(uint32_t), 1, f, &q); 
    FW(&version, sizeof(uint32_t), 1, f, &q);
    FW(&nodeCount, sizeof(uint32_t), 1, f, &q);

    int ID = 0;
    int nextID = 1;
    q_enqueue(&q, g_root, ID); 
    while (!q_empty(&q)) {
        // dequeue parent 
        Node* temp;
        int success = q_dequeue(&q, &temp, &ID);
        if (success != 1) { FAIL(&q, f); }

        // write dequeued node to file
        if (temp != NULL) {
            uint8_t isQuestion = temp->isQuestion;
            FW(&isQuestion, sizeof(uint8_t), 1, f, &q);

            char* text = temp->text;
            if (text == NULL) { FAIL(&q, f); }

            uint32_t textLen = strlen(temp->text);
            FW(&textLen, sizeof(uint32_t), 1, f, &q);
            FW(text, sizeof(char), textLen, f, &q);

            int32_t yesId = (temp->yes != NULL) ? nextID++ : -1;
            FW(&yesId, sizeof(int32_t), 1, f, &q);
            // enqueue yes child
            q_enqueue(&q, temp->yes, yesId);

            int32_t noId = (temp->no != NULL) ? nextID++: -1;
            FW(&noId, sizeof(int32_t), 1, f, &q);
            // enqueue no child
            q_enqueue(&q, temp->no, noId);
        }
    }

    q_free(&q);
    fclose(f);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 28  load_tree
 *
 * Read a file written by save_tree and reconstruct the tree.
 * Validate the magic number.  Read all nodes into a flat array
 * first, then link children in a second pass.
 * Free any existing g_root before installing the new one.
 * Return 1 on success, 0 on any error (free partial allocations).
 * ---------------------------------------------------------------- */
int load_tree(const char *filename) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) { return 0; }

    uint32_t magic;
    uint32_t version;
    uint32_t numNodes;
    FR_NOARRAY(&magic, sizeof(uint32_t), 1, f);
    FR_NOARRAY(&version, sizeof(uint32_t), 1, f);
    FR_NOARRAY(&numNodes, sizeof(uint32_t), 1, f);
    if (magic != MAGIC) {
        fclose(f); 
        return 0; 
    } 
    
    Node* flatArray[numNodes]; // should i be mallocing or do it like this?
    int32_t yesArray[numNodes];
    int32_t noArray[numNodes];

    for (uint32_t i = 0; i < numNodes; i++) {
        uint8_t isQuestion;
        uint32_t textLen;
        FR(&isQuestion, sizeof(uint8_t), 1, f, flatArray, i);
        FR(&textLen, sizeof(uint32_t), 1, f, flatArray, i);
        char text[textLen+1];
        text[textLen] = 0; // set null terminator
        FR(text, sizeof(char), textLen, f, flatArray, i);

        flatArray[i] = (isQuestion == 1) ? create_question_node(text) : create_solution_node(text);
        if (flatArray[i] == NULL) {
            FREEFLAT(flatArray, i);
            fclose(f);
            return 0;
        }

        FR(&(yesArray[i]), sizeof(int32_t), 1, f, flatArray, i);
        FR(&(noArray[i]), sizeof(int32_t), 1, f, flatArray, i);
    }

    for (uint32_t i = 0; i < numNodes; i++) {
        if (flatArray[i]->isQuestion == 1) {
            flatArray[i]->yes = flatArray[yesArray[i]];
            flatArray[i]->no = flatArray[noArray[i]];
        }
    }

    free_tree(g_root);
    g_root = flatArray[0];
    fclose(f);
    return 1;
}
