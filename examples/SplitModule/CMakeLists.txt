
loci_build(A.bc "" A.loci)
loci_build(B.bc "" B.loci)
loci_make_executable(splitModule "" A.bc B.bc)

