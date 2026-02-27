# A Polyhedral Transformer for GCC

Although the GCC has a pass called "Graphite" which does polyhedral
transformations. It is limited by the current pass pipeline of GCC. Certain
optimizations like Loop invariant code motion, PRE restructure the loops so
which is not so amenable for polyhedral tools.

This project tries to build a better polyhedral framework for the GCC intermediate
representation GIMPLE.

# Roadmap

1. Detect SCoPs and transform it into polyhedral representation.

2. Implement polyhedral transformation using the pluto library.

3. Transform back polyhedral representation to gimple.

4. Benchmark SPEC and relevant polyhedral benchmarks, compare with pluto and
   polly.

5. Plan on integrating it with GCC.
