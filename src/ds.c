#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab4.h"

/* ----------------------------------------------------------------
 * ds.c  --  all data structures for the Tech Support Diagnosis Tool
 *
 * Implement every function marked TODO.  The only functions in this
 * entire lab permitted to use recursion are free_tree and count_nodes.
 * Everything else must be iterative.
 * ---------------------------------------------------------------- */


/* ====== Tree nodes ============================================== */

/* TODO 1 */
Node *create_question_node(const char *question) {
    Node* node = (Node*) malloc(sizeof(Node));
    if (node == NULL) { return NULL; }

    node->text = strdup(question);
    if (node->text == NULL) { free(node); return NULL; }

    node->yes = NULL; // yes and no ptrs to be updated after or before question node is created?
    node->no = NULL;
    node->isQuestion = 1;
    return node;
}

/* TODO 2 */
Node *create_solution_node(const char *solution) {
    Node* node = (Node*) malloc(sizeof(Node));
    if (node == NULL) { return NULL; }
    
    node->text = strdup(solution);
    if (node->text == NULL) { free(node); return NULL; }

    node->yes = NULL;
    node->no = NULL;
    node->isQuestion = 0;
    return node;
}

/* TODO 3  (recursion allowed) */
void free_tree(Node *node) {
    // condition to prevent null derefs later
    if (node == NULL) { return; }

    // base case: free text of solution and then solution node itself
    if (node->yes == NULL && node->no == NULL) {
        free(node->text);
        free(node);
        return;
    }
    // recursive call on yes and no subtrees
    free_tree(node->yes);
    free_tree(node->no);
    free(node->text);
    free(node);
}

/* TODO 4  (recursion allowed) */
int count_nodes(Node *root) {
    // needed in order for the next condition to run because root is derefed
    if (root == NULL) { return 0; }

    // base case: child node (solution leaf)
    if (root->yes == NULL && root->no == NULL) {
        return 1;
    }
    // recursive call on yes and no subtrees
    return 1 + count_nodes(root->yes) + count_nodes(root->no);
}


/* ====== FrameStack  (dynamic array, iterative traversal) ======== */

/* TODO 5 */
void fs_init(FrameStack *s) {
    if (s != NULL) {
        s->frames = (Frame*) malloc(10 * sizeof(Frame));
        if (s->frames == NULL) { return; }
        s->size = 0;
        s->capacity = 10; // default value to begin with
    }  
}

/* TODO 6 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    if (node == NULL || s == NULL || s->frames == NULL) { return; }

    if (s->size == s->capacity) {
        // copy array and increases capacity
        Frame* moreFrames = (Frame*) realloc(s->frames, 2*s->capacity*sizeof(Frame));
        // if realloc failes
        if (moreFrames == NULL) { return; } 
        s->frames = moreFrames;
        s->capacity *= 2;
    }
    s->frames[s->size].node = node;
    s->frames[s->size].answeredYes = answeredYes;
    s->size++;
}

/* TODO 7 */
Frame fs_pop(FrameStack *s) {
    // if framestack is null or if framestack is empty
    if (s == NULL || s->size <= 0) { return (Frame) {NULL, -1}; }
    // if the frames array ptr is null
    if (s->frames == NULL) { return (Frame) {NULL, -1}; }

    s->size--; 
    Frame popped = (s->frames)[s->size];
    return popped;
}

/* TODO 8 */
int fs_empty(FrameStack *s) {
    return (s == NULL || s->size <= 0) ? 1 : 0;
}

/* TODO 9 */
void fs_free(FrameStack *s) {
    if (!(s == NULL || s->frames == NULL)) {
        free(s->frames); // free array of frames
        s->size = 0; // zero out the other fields of s
        s->capacity = 0;
    }
}


/* ====== EditStack  (dynamic array, undo/redo) =================== */

/* TODO 10 */
void es_init(EditStack *s) {
    if (s != NULL) {
        s->edits = (Edit*) malloc(10 * sizeof(Edit));
        if (s->edits == NULL) { return; }
        s->size = 0;
        s->capacity = 10; // default value to begin with
    }  
}

/* TODO 11 */
void es_push(EditStack *s, Edit e) {
    if (s == NULL || s->edits == NULL) { return; }

    if (s->size == s->capacity) {
        // copy array and increases capacity
        Edit* moreEdits = (Edit*) realloc(s->edits, 2*s->capacity*sizeof(Edit));
        // if realloc fails
        if (moreEdits == NULL) { return; } 
        s->edits = moreEdits;
        s->capacity *= 2;
    }
    s->edits[s->size] = e;
    s->size++;
}

/* TODO 12 */
Edit es_pop(EditStack *s) {
    Edit dummy = {0};
    // if edistack is null or if editstack is empty
    if (s == NULL || s->size <= 0) { return dummy; }
    // if the edits array ptr is null
    if (s->edits == NULL) { return dummy; }

    s->size--; 
    Edit popped = s->edits[s->size];
    return popped;
}

/* TODO 13 */
int es_empty(EditStack *s) {
    return (s == NULL || s->size <= 0) ? 1 : 0;
}

/* TODO 14 */
void es_clear(EditStack *s) {
    if (s != NULL) {
        s->size = 0;
    }
}

/* provided -- do not modify */
void es_free(EditStack *s) {
    free(s->edits);
    s->edits    = NULL;
    s->size     = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) { es_free(s); }


/* ====== Queue  (linked list, BFS) ============================== */

/* TODO 15 */
void q_init(Queue *q) {
    if (q != NULL) {
        q->front = NULL;
        q->rear = NULL;
        q->size = 0;
    }
}

/* TODO 16 */
void q_enqueue(Queue *q, Node *node, int id) {
    if (q != NULL) {
        QueueNode* newRear = (QueueNode*) malloc(sizeof(QueueNode));
        if (newRear == NULL) { return; }
        newRear->treeNode = node;
        newRear->id = id;
        newRear->next = NULL;

        // if there is no current rear (and front)
        if (q->rear == NULL) { 
            q->front = newRear;
            q->rear = newRear;
            q->size++;
            return;
        }
        // when there is an existing rear
        q->rear->next = newRear;
        q->rear = newRear;
        q->size++;
    }
}

/* TODO 17 */
int q_dequeue(Queue *q, Node **node, int *id) {
    if (q == NULL || q->front == NULL || q->size <= 0) { return 0; }

    *node = q->front->treeNode;
    *id = q->front->id;
    QueueNode* oldFront = q->front;
    q->front = oldFront->next;
    free(oldFront);
    q->size--;

    // special case when dequeueing from previous queue of size 1
    if (q->size == 0) {
        q->rear = q->front;
    }
    return 1;
}

/* TODO 18 */
int q_empty(Queue *q) {
    return (q == NULL || q->size <= 0) ? 1 : 0;
}

/* TODO 19 */
void q_free(Queue *q) {
    if (q != NULL && q->front != NULL) {
        QueueNode* curr = q->front;
        while (curr != NULL) {
            QueueNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
        q->front = NULL;
        q->rear = NULL;
        q->size = 0;
    }
}


/* ====== Hash table  (separate chaining) ======================== */

/* TODO 20
 * Convert a string to a canonical key:
 *   letters  -> lowercase
 *   spaces   -> underscore
 *   anything else -> drop
 * Caller owns the returned string and must free() it.
 */
char *canonicalize(const char *s) {
    if (s == NULL) return strdup("");
    
    // key at most will be as long as original string
    char* filtered = (char*) malloc((strlen(s)+1) * sizeof(char));
    if (filtered == NULL) { return NULL; }

    int fIndex = 0;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == ' ') { 
            filtered[fIndex] = '_';
            fIndex++;
            continue;
        }
        if (s[i] >= 'A' && s[i] <= 'Z') {
            filtered[fIndex] = s[i] + 32;
            fIndex++;
            continue;
        }
        if (s[i] >= 'a' && s[i] <= 'z') {
            filtered[fIndex] = s[i];
            fIndex++;
            continue;
        }
    }
    filtered[fIndex] = 0; // add the null terminator
    fIndex++; // now fIndex has total size including null terminator
    char* temp = realloc(filtered, fIndex*sizeof(char));
    if (temp == NULL) {
        free(filtered);
        return NULL;
    }
    filtered = temp;
    return filtered;
}

/* TODO 21  (djb2: hash = hash*33 + c, seed 5381) */
unsigned h_hash(const char *s) {
    if (s == NULL) { return 0; } // error code when string ptr is null
    unsigned hash = 5381;
    for (int i = 0; i < strlen(s); i++) {
        hash *= 33;
        hash += s[i];
    }
    return hash;
}

/* TODO 22 */
void h_init(Hash *h, int nbuckets) {
    if (h == NULL) { return; }
    // buckets is a ptr to an array of entry ptrs
    h->buckets = (Entry**) malloc(nbuckets * sizeof(Entry*));
    if (h->buckets == NULL) { return; }
    // set all of the entry ptrs in array to null
    for (int i = 0; i < nbuckets; i++) {
        h->buckets[i] = NULL;
    }
    h->nbuckets = nbuckets;
    h->size = 0;
}

/* TODO 23 */
int h_put(Hash *h, const char *key, int solutionId) {
    if (h == NULL || key == NULL) { return 0; }

    char* canonicalizedKey = canonicalize(key);
    if (canonicalizedKey == NULL) { return 0; }
    unsigned hashOfStr = h_hash(canonicalizedKey);

    // if key id pair exists already
    if (h_contains(h, key, solutionId) == 1) {
        free(canonicalizedKey);
        return 0;
    }

    int outCount = 0;
    h_get_ids(h, key, &outCount);

    // if key exists, but id doesnt exist, but id list is not empty
    if (outCount != 0) {
        Entry* correctEntry = h->buckets[hashOfStr % h->nbuckets];
        while(strcmp(correctEntry->key, canonicalizedKey) != 0) {
            correctEntry = correctEntry->next;
        }

        if (outCount == correctEntry->vals.capacity) {
            int* newIds = (int*) realloc(correctEntry->vals.ids, 2*correctEntry->vals.capacity*sizeof(int));
            if (newIds == NULL) { return 0; }
            correctEntry->vals.ids = newIds;
            correctEntry->vals.capacity = 2*correctEntry->vals.capacity;

        }

        correctEntry->vals.ids[correctEntry->vals.count++] = solutionId;
        free(canonicalizedKey);
        return 1;
    }

    // if key doesnt even exist 
    Entry* entryToAdd = (Entry*) malloc(sizeof(Entry));
    if (entryToAdd == NULL) {
        free(canonicalizedKey);
        return 0;
    }

    entryToAdd->key = (char*) malloc((strlen(canonicalizedKey)+1) * sizeof(char));
    if (entryToAdd->key == NULL) {
        free(entryToAdd);
        free(canonicalizedKey);
        return 0;
    }
    strcpy(entryToAdd->key, canonicalizedKey);
    entryToAdd->next = NULL;

    entryToAdd->vals.ids = (int*) malloc(5 * sizeof(int));
    if (entryToAdd->vals.ids == NULL) {
        free(entryToAdd->key);
        free(entryToAdd);
        free(canonicalizedKey);
        return 0;
    }
    entryToAdd->vals.count = 0;
    entryToAdd->vals.ids[entryToAdd->vals.count++] = solutionId;
    entryToAdd->vals.capacity = 5;

    // traversal to add the entry back
    Entry* prev = h->buckets[hashOfStr % h->nbuckets];
    if (prev != NULL) {
        while (prev->next != NULL) {
            prev = prev->next;
        }
        prev->next = entryToAdd;
    } else {
        h->buckets[hashOfStr % h->nbuckets] = entryToAdd;
    }
    h->size++;
    free(canonicalizedKey);
    return 1;
}

/* TODO 24 */
int h_contains(const Hash *h, const char *key, int solutionId) {
    if (h == NULL || key == NULL) { return 0; }

    char* canonicalizedKey = canonicalize(key);
    if (canonicalizedKey == NULL) { return 0; }
    unsigned hashOfStr = h_hash(canonicalizedKey);

    Entry* entryToSearch = h->buckets[hashOfStr % h->nbuckets];
    while (entryToSearch != NULL) {
        if (strcmp(canonicalizedKey, entryToSearch->key) != 0) {
            entryToSearch = entryToSearch->next;
        } else {
            break;
        }
    }

    free(canonicalizedKey);
    if (entryToSearch == NULL) {    
        return 0; 
    }

    for (int i = 0; i < entryToSearch->vals.count; i++) {
        if (entryToSearch->vals.ids[i] == solutionId) {
            return 1;
        }
    }
    return 0;    
}

/* TODO 25 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    if (h == NULL || key == NULL) { 
        *outCount = 0;
        return NULL; 
    }

    char* canonicalizedKey = canonicalize(key);
    if (canonicalizedKey == NULL) { 
        *outCount = 0;
        return NULL; 
    }
    unsigned hashOfStr = h_hash(canonicalizedKey);

    Entry* entryToSearch = h->buckets[hashOfStr % h->nbuckets];
    while (entryToSearch != NULL) {
        if (strcmp(canonicalizedKey, entryToSearch->key) != 0) {
            entryToSearch = entryToSearch->next;
        } else {
            break;
        }
    }

    free(canonicalizedKey);
    if (entryToSearch == NULL) { 
        *outCount = 0;
        return NULL; 
    } else {
        *outCount = entryToSearch->vals.count;
        return entryToSearch->vals.ids;
    }
}

/* TODO 26 */
void h_free(Hash *h) {
    if (h != NULL) {
        for (int i = 0; i < h->nbuckets; i++) {
            Entry* entryToFree = h->buckets[i];
            while (entryToFree != NULL) {
                free(entryToFree->key);
                free(entryToFree->vals.ids);
                Entry* prev = entryToFree;
                entryToFree = entryToFree->next;
                free(prev);
            }

        }

        h->nbuckets = 0;
        h->size = 0;
        free(h->buckets);
    }
}
