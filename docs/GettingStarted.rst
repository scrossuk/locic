Getting Started
===============

This document explains how to build/install the Loci Compiler Tools and their dependencies, which should be a fairly simple process. Currently the compiler tools are only known to build in a Unix environment, though given almost all of the code is cross-platform it's unlikely to be difficult to port it to other platforms.

Dependencies
------------

Before building and using the Loci Compiler Tools, it's first necessary to build and install their dependencies. This release depends on the following:

* CMake (build system) - *debian package: cmake, arch package: extra/cmake*
* Boost Filesystem - *debian package: libboost-filesystem-dev, arch package: extra/boost*
* Boost Program Options - *debian package: libboost-program-options-dev, arch package: extra/boost*
* Boost Regex - *debian package: libboost-regex-dev, arch package: extra/boost*
* Boost System - *debian package: libboost-system-dev, arch package: extra/boost*
* LLVM 3.3 to 3.7 (development build) - *debian package: llvm-dev, arch package: extra/llvm*
* Clang 3.3 to 3.7- *debian package: clang, arch package: extra/clang*
* Sphinx (documentation) - *debian package: python-sphinx, arch package: community/python-sphinx*
* tinfo (ncurses library routines) - *debian package: libtinfo-dev, arch package: aur/libtinfo*

Also, the following dependencies are optional:

* LaTeX (documentation PDF output)

Specific versions of LLVM are given since its API tends to be incompatible between releases. The version of Clang used should match the version of LLVM.

The Loci Compiler Tools are written in C++11, and hence must be built by a C++ compiler with C++11 support.

How to build
------------

Once the dependencies have been installed, create a build directory alongside the LOCIC source directory and run:

.. code-block:: bash

	cmake ../locic-src

CMake will output various information about the build configuration; make sure to check the paths it has discovered are correct. Here's an example output:

::

	-- Building Loci Compiler Tools version 1.3.0.0 using build type ''.
	--     Source directory is '/home/user/locic/locic-src'.
	--     Build directory is '/home/user/locic/build'.
	-- LLVM_ROOT_DIR wasn't specified (you can use this to search for LLVM in a particular path).
	-- CLANG_BINARY_NAME wasn't specified (you can use this to search for Clang by a particular name).
	-- CLANG_ROOT_DIR wasn't specified (you can use this to search for Clang in a particular path).
	-- Found LLVM: /usr/lib/llvm-3.5 (found version "3.5") 
	-- Using LLVM 3.5 (name is '3_5').
	--     LLVM binary directory: /usr/lib/llvm-3.5/bin
	--         llc path: /usr/lib/llvm-3.5/bin/llc
	--         llvm-dis path: /usr/lib/llvm-3.5/bin/llvm-dis
	--         llvm-link path: /usr/lib/llvm-3.5/bin/llvm-link
	--         llvm-nm path: /usr/lib/llvm-3.5/bin/llvm-nm
	--         opt path: /usr/lib/llvm-3.5/bin/opt
	--     LLVM include directories: /usr/lib/llvm-3.5/include;/usr/lib/llvm-3.5/build/include
	--         (if this is wrong, specify LOCIC_LLVM_INCLUDE_DIRS)
	--     LLVM library directories: /usr/lib/llvm-3.5/lib
	--         (if this is wrong, specify LOCIC_LLVM_LIBRARY_DIRS)
	-- Found Sphinx: /usr/bin/sphinx-build  
	-- Found Clang: /usr/bin/clang  

You can then just run:

.. code-block:: bash

	make -j

(Note that the ‘-j’ option tells make how many recipes may execute at once; use this to your advantage to reduce the build time on multi-core machines.)

If you want to build Locic with a specific build type (default is Debug; options are Debug, Release, RelWithDebInfo and MinSizeRel), add the relevant CMake variable:

.. code-block:: bash

	cmake -DCMAKE_BUILD_TYPE=Release ../locic-src

Documentation
-------------

The documentation (of which this document is a part) is automatically generated during normal compiler builds for HTML and MAN output formats. You can also enable PDF output (via LaTeX) by specifying *SPHINX_OUTPUT_PDF*:

.. code-block:: bash

	cmake -DSPHINX_OUTPUT_PDF=ON ../locic-src

Examples
--------

The /examples/ subdirectory gives an idea of how to use both the compiler tools and the Loci programming language.

These build with the project and so after building LOCIC you should be able to run these straight away. A good way to learn the language is to modify the examples, re-build (which will just re-build the examples) and then see your modifications in action.

Testing
-------

Various tests are include in the /test/ subdirectory. These tests are not run as part of building the project; they can be run after a successful build with one of the following commands:

.. code-block:: bash

	make test

...or:

.. code-block:: bash

	ctest

If one of the tests fail, run the following command to see the output of all failing tests.

.. code-block:: bash

	ctest --output-on-failure

Building LLVM
~~~~~~~~~~~~~

If you install LLVM from a package manager Locic should be able to find this by searching for llvm-config and using that to get the include directories and libraries for LLVM. In some cases you may want to use your own custom build of LLVM; this section explains how to build LLVM and how to get Locic to build with your custom build of LLVM.

Assuming the following directory structure, where 'llvm-src' contains the source directory tree for the relevant LLVM version:

..

	/ -> llvm -> llvm-src

To build LLVM, you'll typically want to run something like the following commands:

.. code-block:: bash

	pushd llvm
	mkdir llvm-build
	cd llvm-build
	cmake -DCMAKE_BUILD_TYPE=Release ../llvm-src
	make -j
	popd

These commands create an out-of-source build directory in which to build LLVM. They then run CMake with the appropriate flags to create a Release build.

You can now tell Locic where to find your LLVM build by using the *LLVM_ROOT_DIR* variable.

.. code-block:: bash

	pushd locic
	mkdir locic-build
	cd locic-build
	cmake -DLLVM_ROOT_DIR=/path/to/your/llvm/build ../locic-src
	make -j
	popd

You can follow similar steps for Clang by using the *CLANG_ROOT_DIR* variable.

Development
-----------

The Loci Compiler Tools are under active development in `this GitHub repository <https://github.com/scross99/locic>`_. You can checkout the latest version by:

.. code-block:: bash

	git clone https://github.com/scross99/locic.git

You can then follow the :doc:`Getting Started Guide <GettingStarted>` to build the compiler.

The compiler itself also uses the `llvm-abi library <https://github.com/scross99/llvm-abi>`_ for generating functions that conform to platform ABIs (for interoperability with C). This is automatically cloned from GitHub as part of a compiler build, but you can also manually clone it by:

.. code-block:: bash

	git clone https://github.com/scross99/llvm-abi.git

Other pieces of infrastructure for the project:

* `Website <http://loci-lang.org>`_
* `Travis CI <https://travis-ci.org/scross99/locic>`_ - Continuous integration build jobs.
* `Travis CI Artifacts <http://loci-lang.org/travis/>`_ - Artifacts (binaries) from Travis CI jobs.
* `Phabricator <https://locic.exana.io/>`_ - Issue tracker and code review.
* `Twitter account <https://twitter.com/loci_lang>`_
* `Google Groups Mailing List <https://groups.google.com/group/loci-dev>`_

Queries/Suggestions
-------------------

This project is being developed by `Stephen Cross <http://scross.co.uk>`_.

Contributions, queries, suggestions and feedback are all very welcome; you can:

* `Raise an issue on GitHub <https://github.com/scross99/locic/issues>`_
* `Post to the Google Group <https://groups.google.com/group/loci-dev>`_
