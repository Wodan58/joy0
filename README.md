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
CORRECT_GARBAGE_COLLECTOR prevents a segfault when processing the faulty
ack.joy and instead of that prints a runtime error.

Installation
------------

    make
