# gcc-3.4.6-41-compat

This repository is a snapshot of the gcc-3.4.6 respository (one of the last versions of gcc before g77 became obsolete). While the GNU project recommends using gfortran to compile fortran programs, it is sometimes necessary for compatibility reasons to have a version of g77 on your system. The team at Red Hat seems to be keeping the package reasonably up to date with patches, but this is designed for red-hat based systems.

This project will try to keep up with the changes to gcc-3.4 (compatibility version) so that it may be built from source without too much effort. The commits will initially be applying patches and compiling along the way to see how the changes affect the source and the ability of the pakcage to be compiled. This way makes it easier to trace the application of patches to the code.
