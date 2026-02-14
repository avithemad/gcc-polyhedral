# A Polyhedral Transformer for GCC

Although the GCC has a pass called "Graphite" which does polyhedral
transformations. It is limited by the current pass pipeline of GCC. Certain
optimizations like Loop invariant code motion, PRE restructure the loops so
which is not so amenable for polyhedral tools.

This project tries to build a better polyhedral framework for the GCC intermediate
representation GIMPLE.
