=========
sharpSAT
=========

sharpSAT is a #SAT solver based on modern DPLL based SAT solver technology.
This is a new version with several incremental improvements over the 2006
sharpSAT 1.1 which has been published at SAT 2006.

This version also fixed several bugs - most importantly a counting bug,
that causes sharpSAT to report wrong model counts on some instances.

=========
Usage
=========

Usage: sharpSAT [options] [CNF_File]
Options: 
	-noPP	turn off preprocessing
	-noCC	turn off component caching
	-noIBCP	turn off implicit BCP
	-q      quiet mode
	-t [s] 	set time bound to s seconds
	-cs [n]	set max cache size to n MB


=========
Building
=========

It suffices to change to the ./Release subdirectory and run

make


NOTE:

- it is necessary to have at least G++ 4.7 installed for sharpSAT to
compile. This is particularly necessary as we use delegating constructors 

- the GMP bignum package has to be installed in your system



=========
 Changes 
=========

> 12.08 <

- rewrote most of the codebase of the solver, including a redesigned SAT solver
foundation based on more recent developments such as MiniSAT and TiniSAT,
e.g. conflict clause minimization is used now

- disentangled component analysis and the SAT solver part by separating the 
datastructures. Although this imposes a small memory overhead it allows
for simpler and faster algorithms both in the SAT solver and in the component 
analysis part.

- the redesign reduced the number of code lines by more than 30%

- implicit BCP is more efficient now since for every failed literal l found
every literal corresponding to a UIP of the conflict caused by l is asserted
and add a UIP clause for that conflict is added

- changed the decision heursitics to floating point values, and changed 
the VSADS weighting and the activity decay rate.

- As it does not make sense to use only finite precision, the GMP bignumber 
package is now a prerequisite for sharpSAT. This also has to do with the 
fact that we use GMP integer arithmetic now instead of GMP floating points
which is both simpler and has a small memory advantage.

- A new coding of the cached components is introduced which frequently saves 
30 - 50% of cache memory.

- reduced the memory overhead of cached components to less than 40% of the 
original overhead

-  cache size will now be automatically set to at most 95% of the free RAM 
available at program start. This can be overridden, however, by setting the
maximum cache size using the "-cs" option


BUGFIXES

- corrected a bug in the use of the GMP bignumber package that caused (among others)
wrong solution counts reported on bmc-ibm-5, bmc-ibm-10, ra, rb, and rc.

- the solver is fully 64bit compliant now; esepcially the cache size can now be 
larger than GB

- several out-of-memory problems with the old sharpSAT have been resolved
