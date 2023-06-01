Joy0
----

This is the original version of [Joy](https://github.com/Wodan58/Joy),
created by Manfred von Thun. It is kept as a reference implementation
in order to make sure that other implementations don't deviate too much
from this one.

Changes
-------

Some system header files have been added.
The return value of newnode needs to be captured in a variable.
This introduces a sequence point, preventing unspecified behaviour.
TRACING was used to locate the problem, so it was kept in the source code.
CORRECT_GARBAGE_COLLECTOR prints a runtime error in case of memory overflow.
Because the garbage collector uses recursion, a stack overflow is possible.

Warning
-------

The source code assumes that sizeof(long) == sizeof(void *).

Installation
------------

    make

Testing
-------

    cd test2
    for i in *.joy
    do
        ../joy <$i >$i.out
    done
    grep -l false *.out
