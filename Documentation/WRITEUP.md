# ECE 312 Lab 4 Write-Up: Tech Support Diagnosis Tool

**Name:** Kanishk Sama
**EID:** kvs574
**Date:** 4/9/26


## Section 1 — Design Choices (two required, ~100 words each)

For each: what did you choose, what was the alternative, and why?

### 1.A — Array-backed stack vs. linked-list stack

*What I chose:* I chose a dynamic array that doubles in capacity when full, which was used for both the FrameStack and EditStack.

*What I considered instead:* Instead of a dynamic array, a linked-list stack could've been used, where each push onto the stack allocates a new node.

*Why:* In the case of a dynamic array, both push and pop operations have an average time complexity of O(1), and there are more caching benefits because all frames are stored continuously in memory. Using a linked list, one would need to allocate and free memory during each push and pop operation, which introduces additional overhead and increases the chance of memory leaks and memory allocation failures.

---

### 1.B — Two-pass design in `load_tree`

*What I chose:* I chose to implement a two-pass approach where the first pass reads all nodes into a flat array and the second pass links children using the stored yes/no IDs.

*What I considered instead:* Another option would be to link children immediately during the first pass when each node is read.

*Why:* During the first phase (read phase), the children haven't been read from the file yet, so their memory doesn't exist. Attempting to link during read would either require looking ahead in the file or holding pointers to unallocated memory. The flat array approach ensures that all nodes exist before any linking happens, allowing pointer assigning to be safe and clear.

---

## Section 2 — Complexity Analysis (all four required)

Show the reasoning, not just the answer.

### 2.1 — Amortized cost of a single FrameStack push

### 2.2 — Hash table average-case lookup

### 2.3 — Diagnosis traversal (best, worst, average)

### 2.4 — `find_shortest_path` time and space

---

## Section 3 — Bugs (two required)

### 3.A — Undefined behavior when dequeueing from a Queue of size 1

*Symptom:* The pogram crashed or sometimes produced garbage values after dequeuing the last element from the queue, which led BFS traversal to behave incorrectly.

*Cause:*  In `q_dequeue`, after assigning `q->front = oldFront->next`, `q->rear` still pointed to the freed node when the queue becomes empty. Any future enqueue's would then lead to undefined behavior because it would write to a dangling rear pointer.

*Fix:* After freeing `oldFront`, check if `q->front` is now NULL and, if that is the case, set `q->rear = NULL` as well, ensuring both pointers are synced when the queue is empty.

*Rule that would have prevented it:* When a data structure has multiple pointers that should stay consistent with each other, update all of them in every distinct section of the code, not just the one that gets directly updated.

---

### 3.B — Wrong ID used when enqueuing children in `save_tree`

*Symptom:* The saved binary file linked nodes to the wrong children, so loading produced a corrupted/garbage knowledge base with incorrect parent and child relationships.

*Cause:* Children were enqueued with `nextID` after it had already been incremented by the `nextID++` expression, so the ID stored in the queue didn't match the ID written to the file for that child.

*Fix:* Assign `yesId` and `noId` using `nextID++` first, then enqueue with `yesId` and `noId` directly so the enqueued ID matches what was written.

*Rule that would have prevented it:* Never use a variable both as storage and as a counter in the same expression. The correct thing to do is assign value first, then increment, and then use the assigned value consistently.

---

## Section 4 — Knowledge Base Reflection (~100 words)

1. How many nodes does your submitted `techsupport.dat` contain?

2. What categories of problems did you teach the program? Give one example question/solution pair for each category.

3. Look at the tree with `[V]`.  Are the questions you taught it good distinguishing questions — do they split the remaining candidates roughly in half?  Name one question you would improve and describe what you would replace it with.

4. Describe one `[F]ind Path` result.  What were the two solutions, what was the shared path, and did the output match your expectation?

---

## Section 5 — Reflection (3–5 sentences)

Answer at least two:

- What was the hardest part and why?
- What did the iterative diagnosis loop teach you about recursion?
- What would you do differently if starting over?
- Was there a moment something clicked? What was it?

---

## Section 6 — Time Log

| Date | Hours | What you worked on |
|------|-------|--------------------|
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |
| | | |

**Total hours:** ___
