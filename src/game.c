#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab4.h"

extern Node      *g_root;
extern EditStack  g_undo;
extern EditStack  g_redo;
extern Hash       g_index;

/* ----------------------------------------------------------------
 * TODO 31  run_diagnosis
 *
 * Walk the decision tree iteratively (no recursion) using a
 * FrameStack.  At each question node ask the user yes/no and push
 * the appropriate child.  At each solution leaf display the fix and
 * ask whether it solved the problem.
 *
 * If the fix did not help, enter the learning phase:
 *   - Ask the user what would actually fix the problem.
 *   - Ask for a yes/no question that distinguishes their problem
 *     from the solution just shown.
 *   - Ask which answer applies to their problem.
 *   - Create a new question node and a new solution node, wire them
 *     correctly, graft them into the tree, record an Edit for
 *     undo/redo, and index the new question with canonicalize/h_put.
 *
 * Edge case: if parent is NULL the root itself must be replaced.
 * ---------------------------------------------------------------- */
void run_diagnosis(void) {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Tech Support Diagnosis");
    attroff(COLOR_PAIR(5) | A_BOLD);

    mvprintw(2, 2, "I'll help diagnose your tech problem.");
    mvprintw(3, 2, "Answer each question with y or n.");
    mvprintw(4, 2, "Press any key to start...");
    refresh();
    getch();

    FrameStack stack;
    fs_init(&stack);

    /* TODO: implement */

    fs_free(&stack);
}

/* ----------------------------------------------------------------
 * TODO 32  undo_last_edit
 * Return 1 on success, 0 if the undo stack is empty.
 * ---------------------------------------------------------------- */
int undo_last_edit(void) {
    // pop off undo stack
    if (es_empty(&g_undo) == 1) { return 0; }
    Edit lastEdit = es_pop(&g_undo);
    if (lastEdit.parent == NULL) { // no parent node at all so root gets replaced
        g_root = lastEdit.oldLeaf;  
    } else if (lastEdit.wasYesChild) {
        lastEdit.parent->yes = lastEdit.oldLeaf;
    } else {
        lastEdit.parent->no = lastEdit.oldLeaf;
    }
    // push on to redo stack
    es_push(&g_redo, lastEdit);
    return 1;
}

/* ----------------------------------------------------------------
 * TODO 33  redo_last_edit
 * Return 1 on success, 0 if the redo stack is empty.
 * ---------------------------------------------------------------- */
int redo_last_edit(void) {
    // pop off redo stack
    if (es_empty(&g_redo) == 1) { return 0; }
    Edit lastEdit = es_pop(&g_redo);
    if (lastEdit.parent == NULL) {
        g_root = lastEdit.newQuestion;
    } else if (lastEdit.wasYesChild) {
        lastEdit.parent->yes = lastEdit.newQuestion;
    } else {
        lastEdit.parent->no = lastEdit.newQuestion;
    }
    // push on to redo stack
    es_push(&g_undo, lastEdit);
    return 1;
}
