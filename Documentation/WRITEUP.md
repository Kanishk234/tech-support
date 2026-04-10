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

The overall amortized cost of a single FrameStack push is `O(1)`. In most cases, pushing onto the stack requires a simple array access, which has constant runtime. Occassionally, there will be instances where the array will have to expand in capacity, which will result in a realloc that has `O(n)` time. Over the sequence of several push operations, the occassional `O(n)` realloc will spread out over the many `O(1)` pushes, resulting in an amortized cost of `O(1)` for a single FrameStack push.

### 2.2 — Hash table average-case lookup

The average-case of a hash table lookup in `h_contains` and `h_get_ids` is `O(1)`. Both functions first canonicalize the key and compute `h_hash(canonicalizedKey) % h->nbuckets` to find the bucket in constant time. They then traverse the chain in that bucket using a while loop to compare keys until a match is found. The average chain length is n/m where n is the number of keys and m is the number of buckets. The djb2's uniform hashing also tends to prevent clustering, so chains stay short on average, allowing `O(1)` lookup. In the worst case where all keys hash to the same bucket, the chain traversal gets downgraded to `O(n)`.

### 2.3 — Diagnosis traversal (best, worst, average)

### 2.4 — `find_shortest_path` time and space

The time complexity of `find_shortest_path` is `O(n)`. The main cost comes from the two DFS calls. Each call walks the tree looking for a target solution node. In the worst case, the target is the very last node visited, which means the DFS explored all `n` nodes before finding it. Since we call DFS twice, that is `2n` node visits total, which simplifies to `O(n)`. After both paths are found, we scan through them simultaneously to see where they diverge. This scan is bounded by the height of the tree h, since paths can be at most as long as the tree is tall. In a balanced tree h is log n, but in the worst case of a completely skewed tree it could be n. The two paths are at most the height of the tree (generally log n, but worst case n), so traversing the paths will result in `O(log n)` or `O(n)` worst case. Either way, `O(log n)` is dominated by the `O(n)` DFS time complexity. Everything else like the divergence print, and the check for branch direction is constant or constrained by the height of the tree, so the overall time complexity is `O(n)`.

The space complexity is `O(n)`. The two path arrays `pathForSol1` and `pathForSol2` are each allocated with size `count_nodes(g_root)` giving `O(n)` space each. The recursive DFS stack frames adds `O(log n)` space for the recursion depth, which is `O(log n)` for a balanced tree and `O(n)` for the worst case tree. The dominant term is the path arrays at `O(n)` because `n` is greater than `log n`, so overall space is `O(n)`.

---

## Section 3 — Bugs (two required)

### 3.A — Undefined behavior when dequeueing from a Queue of size 1

*Symptom:* The program crashed or sometimes produced garbage values after dequeuing the last element from the queue, which led BFS traversal to behave incorrectly.

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

The submitted `techsupport.dat` contains 33 nodes.

2. What categories of problems did you teach the program? Give one example question/solution pair for each category.

The tree covers two main categories. The network branch handles connectivity issues. For example, the question "Are you connected to Wi-Fi but have no internet?" leads to the solution "Open cmd and run: ipconfig /flushdns, then try browsing again." The non-network branch handles hardware and software issues. For example, "Is the screen flickering or showing artifacts?" leads to a solution of "Check display cable connection or try a different monitor cable."

3. Look at the tree with `[V]`.  Are the questions you taught it good distinguishing questions — do they split the remaining candidates roughly in half?  Name one question you would improve and describe what you would replace it with.

The network side of the tree splits fairly well early on by separating Wi-Fi from wired connections and then DNS issues are split from physical hardware issues. The non-network side is weaker though. The question "Is it a display or screen issue?" is broad and only splits one specific problem off from everything else, leaving a long chain of unrelated problems under the NO branch. A better replacement would be something like "Is the problem hardware-related (monitor, printer, keyboard)?" which would split the remaining candidates more evenly between hardware and software issues.

4. Describe one `[F]ind Path` result.  What were the two solutions, what was the shared path, and did the output match your expectation?

Running `Find Path` between "Run: ipconfig /flushdns" and "Check all ethernet cables, replace if damaged, reseat both ends" showed a shared path through the root question "Is the problem with a network device?" and then "Are you connected to Wi-Fi but have no internet?". The DNS fix was the solution if YES and the ethernet fix was the solution if NO. This matched expectations since both are network problems but one is wireless and one is wired, so they should diverge at this exact question.

---

## Section 5 — Reflection (3–5 sentences)

### What was the hardest part and why?

The hardest part was implementing `save_tree` and `load_tree` correctly. The challenge wasn't the file I/O syntax itself but getting the ID scheme and linking right. Specifically, it was ensuring that the ID assigned to each child node when written to the file exactly matched the ID enqueued into the BFS queue, so that the second pass in `load_tree` could link nodes correctly. A subtle off by one erro in when `nextID` was incremented versus when it was passed to `q_enqueue` caused the entire tree structure to load corrupted, and debugging a binary file with no human-readable format made it difficult to pinpoint the issue.

### What would you do differently if starting over?

I would define a consistent error handling strategy before writing any code rather than adding it incrementally. Most of the bugs I encountered weren't in the core logic because they were in error paths. Some notable examples I had were forgetting to null both `front` and `rear` on last dequeue, not freeing canonicalized strings after `h_put`, and not closing the file on every early return in `save_tree`. Having a checklist of what needs to be cleaned up on failure for each function before writing it would have caught several of these issues immediately instead of during valgrind or other testing.

---

## Section 6 — Time Log

| Date | Hours | What you worked on |
|------|-------|--------------------|
| 3-25-26 | 1.5 | TODOs 1-4 (Tree) |
| 3-26-26 | 2 | TODOs 5-9 (FrameStack) |
| 3-26-26 | 1 | TODOs 10-14 (EditStack) |
| 3-27-26 | 2.5 | TODOs 15-19 (Queue) |
| 3-27-26 | 3.5 | TODOs 20-26 (Hash Table) |
| 3-28-26 | 1 | TODO 29 (Integrity Check) |
| 3-29-26 | 4 | TODOs 27-28 (Persistence) |
| 4-1-26 | 4 | TODO 30 (Shortest Path) |
| 4-3-26 | 1 | TODOs 32-33 (Undo and Redo) |
| 4-3-26 | 3 | TODO 31 (Run Diagnosis) |
| 4-9-26 | 1.5 | Tested the tool and built knowledge base |
| 4-10-26 | 1.5 | Refactored TODO 30 (Shortest Path) to use recursion |
| 4-10-26 | 2 | Completed the write up |

**Total hours:** 28.5
