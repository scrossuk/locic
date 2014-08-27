Getting Started
===============

This document explains how to build/install the Loci compiler tools and its dependencies, which should be a fairly simple process. Currently the compiler tools are only known to build in a Unix environment, though given almost all of the code is cross-platform it's unlikely to be difficult to port it to other platforms.

Dependencies
------------

Before building and using LOCIC, it's first necessary to build and install its dependencies. This release of LOCIC depends on the following:

* CMake (build system) - *debian package: cmake*
* Boost Filesystem - *debian package: libboost-filesystem-dev*
* Boost Program Options - *debian package: libboost-program-options-dev*
* Boost Regex - *debian package: libboost-regex-dev*
* Boost System - *debian package: libboost-system-dev*
* LLVM 3.3/3.4/3.5 (build with instructions below)
* Sphinx (documentation) - *debian package: python-sphinx*

Also, the following dependencies are optional:

* LaTeX (documentation PDF output)

Specific versions of LLVM are given since its API tends to be incompatible between releases.

LOCIC is written in C++11, and hence must be built by a C++ compiler with C++11 support.

Building LLVM
~~~~~~~~~~~~~

Default Ubuntu packages tend not to include the LLVMConfig.cmake file, which LOCIC relies on to discover information about the LLVM build (such as the include directories). It's therefore recommended to download the latest version of LLVM and build it using CMake.

Assuming the following directory structure, where 'llvm-src' contains the source directory tree for the relevant LLVM version:

..

	/ -> llvm -> llvm-src

To build LLVM, you'll typically want to run something like the following commands:

.. code-block:: bash

	pushd llvm
	mkdir llvm-build
	cd llvm-build
	cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release ../llvm-src
	make -j
	popd

These commands create an out-of-source build directory in which to build LLVM. They then run CMake with the appropriate flags to create a Release build that will be installed to '/usr' on the host system.

(Note that the ‘-j’ option tells make how many recipes may execute at once; use this to your advantage to reduce the build time on multi-core machines.)

It's not required for building LOCIC, but at this point you can install LLVM into your system directories by running the following command **as root**:

.. code-block:: bash

	make install

How to build
------------

Once the dependencies have been installed, create a build directory alongside the LOCIC source directory and run:

.. code-block:: bash

	cmake ../locic-src

You can then just run:

.. code-block:: bash

	make

If you're building against an LLVM build that you haven't installed, you can use CMAKE_PREFIX_PATH to find it:

.. code-block:: bash

	cmake -DCMAKE_PREFIX_PATH=/path/to/llvm/build ../locic-src

The CMake script will show you the information it found for LLVM (or fail if it couldn't find the LLVMConfig.cmake file). For LLVM versions **before 3.5** you'll probably find that the include directories and library directories are **incorrect**. In this case, you should specify these manually:

.. code-block:: bash

	export LLVM_SOURCE=/path/to/llvm/source
	export LLVM_BUILD=/path/to/llvm/build
	cmake -DCMAKE_PREFIX_PATH="$LLVM_BUILD" -DLOCIC_LLVM_INCLUDE_DIRS="$LLVM_SOURCE/include;$LLVM_BUILD/lib" -DLOCIC_LLVM_LIBRARY_DIRS="$LLVM_BUILD/lib" ../locic-src

Documentation
-------------

The documentation (of which this document is a part) is automatically generated during normal compiler builds for HTML and MAN output formats. You can also enable PDF output (via LaTeX) by specifying *SPHINX_OUTPUT_PDF*:

.. code-block:: bash

	cmake -DSPHINX_OUTPUT_PDF=ON ../locic-src

Examples
--------

The /examples/ subdirectory gives an idea of how to use both the compiler tools and the Loci programming language.

These build with the project and so after building LOCIC you should be able to run these straight away. A good way to learn the language is to modify the examples, re-build LOCIC (which will just re-build the examples) and then see your modifications in action.

Testing
-------

Various tests are include in the /test/ subdirectory. These
tests are not run as part of building the project; they
can be run after a successful build with one of the following
commands:

.. code-block:: bash

	make test

...or:

.. code-block:: bash

	ctest

If one of the tests fail, run the following command to see the
output of all failing tests.

.. code-block:: bash

	ctest --output-on-failure

