#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab4.h"

extern Node *g_root;
typedef struct PathNode {
    Node *node;       
    int   parentIdx;  // bfs index of this node's parent (-1 for root)
} PathNode;

/* ----------------------------------------------------------------
 * TODO 29  check_integrity
 *
 * Use BFS to verify:
 *   - Every question node has both yes and no children (non-NULL).
 *   - Every solution node has both children NULL.
 * Return 1 if valid, 0 if any violation is found.
 * ---------------------------------------------------------------- */
int check_integrity(void) {
    if (g_root == NULL) { return 1; }
    Queue q;
    q_init(&q);
    q_enqueue(&q, g_root, 0); // idk what to put for id

    while (!q_empty(&q)) {
        // dequeue parent 
        Node* temp;
        int tempId;
        int success = q_dequeue(&q, &temp, &tempId); // again idk what id to put
        if (success == 0) { /*what do i do*/ }

        // check its integrity
        if (temp != NULL) {
            if (temp->isQuestion == 1) {
                if (temp->yes == NULL || temp->no == NULL) {
                    q_free(&q);
                    return 0;
                }
            } else {
                if (temp->yes != NULL || temp->no != NULL) {
                    q_free(&q);
                    return 0;
                }
            }

            // enqueue children
            q_enqueue(&q, temp->yes, 0);
            q_enqueue(&q, temp->no, 0);
        }
    }

    q_free(&q);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 30  find_shortest_path
 *
 * Given the exact text of two solution leaves, display the
 * questions that distinguish them.  Use BFS with a parent-tracking
 * PathNode array to find both leaves, build ancestor arrays for
 * each, find the Lowest Common Ancestor (LCA), then print:
 *   - The shared path of questions both solutions pass through.
 *   - The divergence question (LCA) and which branch leads where.
 *
 * Display results with mvprintw.  Print an error if either
 * solution is not found.  Free all allocations before returning.
 * ---------------------------------------------------------------- */
void find_shortest_path(const char *sol1, const char* sol2) {
    int row = 10;
    if (g_root == NULL || sol1 == NULL || sol2 == NULL) {
        mvprintw(row, 2, "Error: one or both solutions not found.");
        refresh();
        return;
    }

    int size = count_nodes(g_root);
    Node* pathForSol1[size];
    Node* pathForSol2[size];
    int len1 = 0; int len2 = 0;

    if (dfs(g_root, sol1, pathForSol1, &len1) == 0) {
        mvprintw(row, 2, "Error: one or both solutions not found.");
        refresh();
        return;
    }
    if (dfs(g_root, sol2, pathForSol2, &len2) == 0) {
        mvprintw(row, 2, "Error: one or both solutions not found.");
        refresh();
        return;
    }

    int loopLen = (len1 < len2) ? len1 : len2;
    int divergencePoint = loopLen-1;
    for (int i = 0; i < loopLen; i++) {
        if (strcmp(pathForSol1[i]->text, pathForSol2[i]->text) != 0) {
            divergencePoint = i-1;
            break;
        }
    }

    // header
    mvprintw(row++, 2, "Distinguishing path between:");
    mvprintw(row++, 4, "A: \"%s\"", sol1);
    mvprintw(row++, 4, "B: \"%s\"", sol2);
    row++;

    // shared path
    mvprintw(row++, 2, "Shared path (both solutions pass through):");
    for (int i = 0; i < divergencePoint; i++) {
        mvprintw(row++, 4, "%s", pathForSol1[i]->text);
    }
    row++;

    // divergence point
    Node* LCA = pathForSol1[divergencePoint];
    mvprintw(row++, 2, "Divergence point (LCA):");
    mvprintw(row++, 4, "%s", LCA->text);
    if (pathForSol1[divergencePoint+1] == LCA->yes) {
        mvprintw(row++, 6, "YES -> \"%s\"", sol1);
        mvprintw(row++, 6, "NO  -> \"%s\"", sol2);
    } else {
        mvprintw(row++, 6, "YES -> \"%s\"", sol2);
        mvprintw(row++, 6, "NO  -> \"%s\"", sol1);
    }
    refresh();
}

int dfs(Node* node, const char* target, Node* path[], int* len) {
    if (node == NULL) {
        return 0;
    }

    path[*len] = node;
    (*len)++;
    if (node->isQuestion == 0 && strcmp(target, node->text) == 0) {
        return 1;
    } else if (dfs(node->yes, target, path, len) == 1) {
        return 1;
    } else if (dfs(node->no, target, path, len) == 1) {
        return 1;
    }

    (*len)--; // backtrack the length
    return 0;
}
