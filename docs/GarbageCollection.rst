Garbage Collection
==================

See :doc:`Heap Memory Management <HeapMemoryManagement>`.

Many languages now include garbage collection, including high-level scripting languages such as Python as well as slightly lower-level languages such as Java and C#. C and C++ have not adopted garbage collection as a core language feature, focusing instead on manual memory management and smart pointers; D is a very low-level language that does include garbage collection.

Loci combines garbage collection with the memory management approaches taken by C++, so that developers can choose the allocation method that is appropriate to their application. The aim is to make it possible to use manual memory management, smart pointers and deterministic destruction alongside garbage collected data.

Again, this is divided between 'system' modules and PODs, in which modules benefit greatly from stack allocation or heap allocation with management by smart pointers, whereas PODs benefit from being collected automatically.

In the case of module objects, it is typical to only have a single non-copyable instance of the object, which needs deterministic destruction so it can release its non-memory resources, and therefore tends to be ideally suited to being placed on the stack.

If a module object is allocation on the heap, a unique smart pointer (i.e. where there is one owner) is almost always desired to manage it. If there need to be multiple owners, a reference counted smart pointer can be used.

PODs on the other hand are typically immutable (or at least, rarely modified) and certainly copyable objects, but for which it is ideal to avoid copying. If these objects are allocated on the heap, it can be difficult to manage them because many objects simultaneously own them. Garbage collection is the ideal solution in this case.

A good example of effective use of garbage collection are ropes used in the implementation of Loci's :doc:`Strings <Strings>`. These are binary trees, of which the nodes are concatenations of their children, and the leaves refer to positions within blocks.

Pointers to the nodes, the leaves and the blocks they refer to can be freely copied, eliminating the performance cost of copying, with the knowledge that the garbage collector will reclaim the memory when necessary. Ropes thereby achieve logarithmic complexity (as opposed to linear) for concatenation, insertion and deletion of strings.

This can be achieved by reference counting, but the increments and decrements to the counters are costly compared to the simple pointer copies allowed by garbage collection.

