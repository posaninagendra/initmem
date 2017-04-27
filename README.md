# initmem 

Compilers main function is to down convert the high level language to lower level machine codes and create the executable. GCC (GNU Compiler Collection) is one of the most popular compilers. These compilers not only do the down conversion but they also does a lot of omptimizations that speeds up the execution of the programs. Often, these compilers fail to address all the programming needs, in order to overcome these problems they provide extensions through compiler plugins. GCC falls in this class of compilers that provides extensions through its plugin interface.

*initmem* is a GCC compiler plugin that finds the uninitialized variables in a given program and initializes them to zero at compilation time. It solves the problem of data leaks through memory reads of uninitialized variables. 

### Installation

GCC is an old compiler, it first released in 1987 (source: [Wikipedia](https://en.wikipedia.org/wiki/GNU_Compiler_Collection)). It has evolved over the years to gain new features. One of these functionalities is being extensible via plugins. Plugins let us to extend the compiler without getting ourselves too dirty. GCC codebase, after 30 years, is huge and comes with its own quirks, so writing a plugin is not trivial but, in my opinion, can be very rewarding and revealing on how a real compiler works (for good and bad, of course). 

The plugin interface of GCC follows a model similar to that of Linux modules: API stability is not guaranteed between versions. So for this plugin we are using GCC 5.2, to install the plugin we need to setup GCC 5.2, here are the steps that guide you to install GCC 5.2. I used the installation guidelines provided by Roger in one of his articles on [How to write GCC plugins](http://thinkingeek.com/2015/08/16/a-simple-plugin-for-gcc-part-1/). 

1. Download GCC 5.2

```shell
# define a variable BASEDIR that we will use all the time
$ export BASEDIR=$HOME/gcc-plugins
# Create the directory, if it does not exist
$ mkdir -p $BASEDIR
# Enter the new directory
$ cd $BASEDIR
# Download gcc using 'wget' ('curl' can be used too)
$ wget http://ftp.gnu.org/gnu/gcc/gcc-5.2.0/gcc-5.2.0.tar.bz2
# Unpack the file
$ tar xfj gcc-5.2.0.tar.bz2
```

2. The next step is to get some software required by GCC itself. 

```shell
# Enter in the source code directory of GCC
$ cd gcc-5.2.0
# And now download the prerequisites
$ ./contrib/download_prerequisites
```

3. Configure and build the compiler

```shell
# We are in gcc-5.2.0, go up one level
$ cd ..
# Now create the build directory, gcc-build is a sensible name
$ mkdir gcc-build
# Enter the build directory
$ cd gcc-build
# Define an installation path, it must be an absolute path!
# '$BASEDIR/gcc-install' seems an appropiate place
$ export INSTALLDIR=$BASEDIR/gcc-install
# Configure GCC
$ ../gcc-5.2.0/configure --prefix=$INSTALLDIR --enable-languages=c,c++
# Build 'getconf _NPROCESSORS_ONLN' will return the number of threads
# we can use, in order to build GCC in parallel
$ make -j$(getconf _NPROCESSORS_ONLN)
```

4. Install

```shell
make install
```

5. Check installation

```shell
# Create a convenience variable for the path of GCC
$ export GCCDIR=$INSTALLDIR/bin
$ $GCCDIR/g++ --version
g++ (GCC) 5.2.0
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
