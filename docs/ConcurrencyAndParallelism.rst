Concurrency and Parallelism
===========================

Recently there have been many discussions about taking advantage of the increasing amounts of parallelism that is becoming possible through improvements to hardware. Much focus is directed towards programming languages and how they might facilitate taking advantage of these improvements.

In particular, a compiler can struggle to optimise normal code in a language such as in C into machine code that can be effectively parallelised by the hardware. :doc:`Vector Types <VectorTypes>` and operations are a good method to achieve performance benefits from new architectures within the context of a single processor.

However, there is of course no silver bullet to achieving multi-processor performance benefits; developers must still be responsible for profiling their own code and restructuring it so that it can be run in parallel.

Typically, gains are made either in low-level routines (e.g. image processing) via parallelising instructions, or via high-level restructuring. In the former case, the language provides vector operations that need to be used correctly; in the latter it is up to the developer to implement their high-level design, though the standard library should assist this.

Some forms of parallelism, such as threads, can be dangerous since it is difficult for the programmer to analyse how parallel threads might interact. Processes, on the other hand, are an excellent way to achieve parallelism since memory is no longer shared, but may suffer from a lack of system support or high IPC overhead.

Again, it is up to the developer to make and implement these decisions, so like any other language Loci cannot provide significant 'automatic' parallelisation of code without substantial cooperation from the developer of the code.

