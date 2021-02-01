CortexSuite
-----------

CortexSuite integrates a new set of benchmarks with the vision benchmarks from SD-VBS.
It's a little crufty, but basically you run them separately. Apologies.

In most cases, you will not be using our Makefile scaffolding, since you are not running on X86.

But here is a description of how it works.

A. **SD-VBS Computer Vision Benchmark Subset of CortexSuite ("vision")**:

Enter the vision directory and follow the instructions there.

B. **Non-computer vision subset ("cortex")**:

Compile the non-computer vision benchmarks with the command:

_make compile-cortex_

This will compile the benchmarks for the small,medium,and large benchmarks
where applicable. 

To run a specific set of benchmarks use the commands

_make run-small_

_make run-medium_

_make run-large_

To gather the cycle counts for all of these algorithms run

_make cycles-cortex_

This will print the cycle count for small, medium, and large datasets if applicable
for each algorithm

C. **As of 5/2/2020, four of the benchmarks have been fixed (kmeans clustering, spectral clustering, CNN, word2vec).**

   1. we have enabled both kmeans and spectral clustering, under the clustering
      directory; which are accessible from the top level makefile

   2. we have fixed the CNN benchmark makefile that prevented compilation

   3. we have fixed the word2vec benchmark that prevent compilation

   4. CNN and word2vec contain both training and inference. however, because of run times, CNN and word2vec
      do not have a standard small/medium/large configuration.
      So using them will require you to determine what you want to test in terms of training vs. inference.
      It may be worth it, since these are close to end-to-end workloads.

   5. If somebody would like to do a pull request to separate these into a small/medium/large input size
      and training versus inference, we would gladly consider integrating it.

D. **Everything compiles on Centos 7.6. More modern versions of the compilers may require some fixes. We accept pull requests!**

F. **If you have contributed to CortexSuite, please feel free to add your names to the contributors.txt file!**

G. **See the cortexsuite website here: http://cseweb.ucsd.edu/groups/bsg/**
