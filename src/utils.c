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
void find_shortest_path(const char *sol1, const char *sol2) {
    if (g_root == NULL) {
        mvprintw(10, 2, "Error: knowledge base is empty.");
        refresh();
        return;
    } else if (sol1 == NULL || sol2 == NULL ) {
        mvprintw(10, 2, "Error: one or both solutions not found.");
        refresh();
        return;
    }  

    PathNode bfsNodes[count_nodes(g_root)];
    int bfsCount = 0;
    int parentIndex = -1; // signalling no parent
    int solIndex1  = -1;
    int solIndex2 = -1;
    Queue q;
    q_init(&q);

    q_enqueue(&q, g_root, parentIndex);
    while (!q_empty(&q)) {
        Node* temp;
        if (q_dequeue(&q, &temp, &parentIndex) != 0 && temp != NULL) {
            bfsNodes[bfsCount].node = temp;
            bfsNodes[bfsCount].parentIdx = parentIndex;

            // checking that temp node is a solution node
            if (temp->yes == NULL && temp->no == NULL) {
                if (strcmp(temp->text, sol1) == 0) {
                    solIndex1 = bfsCount;
                }
                if (strcmp(temp->text, sol2) == 0) {
                    solIndex2 = bfsCount;
                }
            }

            parentIndex = bfsCount;
            q_enqueue(&q, temp->yes, parentIndex);
            q_enqueue(&q, temp->no, parentIndex);
            bfsCount++;
        }
    }

    if (solIndex1 == -1 || solIndex2 == -1) {
        mvprintw(10, 2, "Error: one or both solutions not found.");
        refresh();
        return;
    }

    Node* ancestors1[bfsCount];
    Node* ancestors2[bfsCount];
    int ancestors1Count = 0;
    int ancestors2Count = 0;

    // create ancestor array by starting from solution and linking our way back to root
    for (int i = 0; solIndex1 != -1; i++) {
        ancestors1[i] = bfsNodes[solIndex1].node;
        solIndex1 = bfsNodes[solIndex1].parentIdx;
        ancestors1Count++;
    }
    for (int i = 0; solIndex2 != -1; i++) {
        ancestors2[i] = bfsNodes[solIndex2].node;
        solIndex2 = bfsNodes[solIndex2].parentIdx;
        ancestors2Count++;
    }

    // reverse both ancestor arrays to go from root to solution
    int left = 0;
    int right = ancestors1Count-1;
    while (left <= right) {
        Node* n = ancestors1[left];
        ancestors1[left] = ancestors1[right];
        ancestors1[right] = n;
        left++; right--;
    }
    left = 0;
    right = ancestors2Count-1;
    while (left <= right) {
        Node* n = ancestors2[left];
        ancestors2[left] = ancestors2[right];
        ancestors2[right] = n;
        left++; right--;
    }

    // find LCA of the two paths
    Node* LCA = ancestors1[0]; // root node
    int LCAIndex = 0;
    for (int i = 1; i < ((ancestors1Count > ancestors2Count) ? ancestors2Count : ancestors1Count); i++) {
        if (strcmp(ancestors1[i]->text, ancestors2[i]->text) != 0) {
            LCA = ancestors1[i-1];
            LCAIndex = i-1;
            break;
        }
    }

    // printing the shared paths and divergence point to the screen
    int row = 10;
    mvprintw(row++, 2, "Shared path:");
    for (int i = 0; i <= LCAIndex; i++) {
        mvprintw(row++, 4, "%s", ancestors1[i]->text);
    }
    row++;
    mvprintw(row++, 2, "Divergence at: %s", LCA->text);
    if (ancestors1[LCAIndex+1] == LCA->yes) {
        mvprintw(row++, 4, "YES -> leads to \"%s\"", sol1);
        mvprintw(row++, 4, "NO  -> leads to \"%s\"", sol2);
    } else {
        mvprintw(row++, 4, "YES -> leads to \"%s\"", sol2);
        mvprintw(row++, 4, "NO  -> leads to \"%s\"", sol1);
    }
    refresh();
}
