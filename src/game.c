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
    index_tree(g_root);

    /* TODO: implement */
    fs_push(&stack, g_root, -1);

    Node* parent = NULL;
    int parentAnsweredYes = -1;

    int row = 6;
    while (fs_empty(&stack) != 1) {
        clear();
        row = 2;
        Frame popped = fs_pop(&stack);
        Node* curr = popped.node;
        if (curr == NULL) { continue; }

        if (curr->isQuestion == 1) { // improve error checking
            // printing but need to check that it does it right
            mvprintw(2, 2, "%s", curr->text);
            refresh();
            // get yes/no from user
            int answer = get_yes_no(4, 2, "Your answer (y/n): ");
            // push yes or no child accordingly
            Node* child = (answer == 1) ? curr->yes : curr->no;
            fs_push(&stack, child, answer);
            // update parent tracking
            parent = curr;
            parentAnsweredYes = answer;
        } else {
            mvprintw(row, 2, "Suggested fix: %s", curr->text);
            row++;
            int solved = get_yes_no(row, 2, "Did this solve your problem? (y/n): ");
            row++;
            if (solved == 1) { 
                fs_free(&stack); 
                return; 
            }

            // teach the tree a new thing
            // get new solution text from user
            char* input = get_input(row, 2, "What would fix this problem? ");
            if (input == NULL || strcmp(input, "") == 0) { fs_free(&stack); return; }
            char newSolution[256]; // 255 char limit
            strncpy(newSolution, input, 255);
            newSolution[255] = 0; // setting null terminator
            row++;

            // get distinguishing question from user
            input = get_input(row, 2, "Give me a yes/no question that distinguishes your problem: ");
            if (input == NULL || strcmp(input, "") == 0) { fs_free(&stack); return; }
            char newQuestion[256];
            strncpy(newQuestion, input, 255);
            newQuestion[255] = 0;
            row++;

            // check if question already exists in hash table
            char *words = strdup(newQuestion);
            char *tokens = strtok(words, " ");
            int duplicate = 0;
            while (tokens != NULL) {
                char *canon = canonicalize(tokens);
                if (canon != NULL && h_get_ids(&g_index, canon, &(int){0}) != NULL) {
                    duplicate = 1;
                    free(canon);
                    break;
                }
                free(canon);
                tokens = strtok(NULL, " ");
            }
            free(words);
            if (duplicate) {
                mvprintw(row, 2, "A similar question already exists. Try rephrasing.\n\n  Press any key to go back to the home screen...");
                refresh();
                getch();
                fs_free(&stack);
                return;
            }

            // get which answer applies to user's problem
            int userAnswer = get_yes_no(row, 2, "For your problem, is the answer yes or no? (y/n): ");
            row++;

            // create new nodes
            Node* newSolutionNode = create_solution_node(newSolution);
            if (newSolutionNode == NULL) { fs_free(&stack); return; }

            Node *newQuestionNode = create_question_node(newQuestion);
            if (newQuestionNode == NULL) { free(newSolutionNode); fs_free(&stack); return; }

            // connect new solution into the question node based on yes or no answer
            if (userAnswer == 1) {
                newQuestionNode->yes = newSolutionNode;
                newQuestionNode->no = curr;
            } else {
                newQuestionNode->yes = curr;
                newQuestionNode->no = newSolutionNode;
            }

            // insert that new question node into the existing tree (knowledge base)
            if (parent == NULL) {
                g_root = newQuestionNode;
            } else if (parentAnsweredYes == 1) {
                parent->yes = newQuestionNode;
            } else {
                parent->no = newQuestionNode;
            }

            // build and push the new change to the tree on to the undo stack
            Edit e;
            e.type = EDIT_INSERT_SPLIT;
            e.parent = parent;
            e.wasYesChild = parentAnsweredYes;
            e.oldLeaf = curr;
            e.newQuestion = newQuestionNode;
            e.newLeaf = newSolutionNode;
            es_push(&g_undo, e);

            // clear redo stack since new edit overrides redo history
            es_clear(&g_redo);

            // index new question words in hash table
            // int newLeafId = 0;
            // assign ID by counting nodes up to newSolutionNode (simple approach: use bfsCount)
            char *qwords = strdup(newQuestion);
            char *tok = strtok(qwords, " ");
            while (tok != NULL) {
                char *canon = canonicalize(tok);
                if (canon != NULL) {
                    h_put(&g_index, canon, count_nodes(g_root));
                    free(canon);
                }
                tok = strtok(NULL, " ");
            }
            free(qwords);

            fs_free(&stack);
            return;
        }
    }
    fs_free(&stack);
}

// helper function to index the tree
void index_tree(Node *node) {
    if (node == NULL) return;
    if (node->isQuestion == 1) {
        char *words = strdup(node->text);
        if (words == NULL) return;
        char *tok = strtok(words, " ");
        while (tok != NULL) {
            char *canon = canonicalize(tok);
            if (canon != NULL) {
                h_put(&g_index, canon, count_nodes(node));
                free(canon);
            }
            tok = strtok(NULL, " ");
        }
        free(words);
        index_tree(node->yes);
        index_tree(node->no);
    }
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
