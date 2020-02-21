 #!/bin/bash

CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-'Debug'}
BUILDDIR=${BUILDDIR:-"build"}

set -E
set +v

while [[ "$1" != "" ]]; do
  case "$1" in
    "--")
        # Stop parsing arguments and let cmake take the rest
        shift
        break
        ;;
    "--scriptdebug")
        set -v
        ;;
    "--clean"|"-c")
        if [ -d "$BUILDDIR" ]; then
            echo "Removing $(pwd)/${BUILDDIR}/"
            rm -r "$(pwd)/${BUILDDIR}"
        else
            echo "No $(pwd)/${BUILDDIR}/ to remove"
        fi
        ;;
    "--release"|"-r")
        echo " CMAKE BUILD TYPE -> RELEASE"
        CMAKE_BUILD_TYPE="Release"
        ;;
    "--debug"|"-d")
        echo " CMAKE BUILD TYPE -> DEBUG"
        CMAKE_BUILD_TYPE="Debug"
        ;;
    "--testing")
        CMAKE_BUILD_TYPE="Debug"
        CMAKE_CONFIGURE_OPTS="$CMAKE_CONFIGURE_OPTS -DBUILD_TESTS:BOOL=ON"
        ;;
    "--nprocs"|"--nproc"|"-j")
        TABTEAHYNW="$1"
        shift
        if [[ "$1" != "" ]]; then
            nprocs="$1"
        else
            echo -e " \033[0;35m$TABTEAHYNW\033[0;0m requires a number of threads, like make -j N"
            exit 1
        fi
        ;;
    "--help"|"-h")
        while IFS="" read -r line; do printf '%b\n' "$line"; done << EOF
Cmake project build script for linux / compatible Unix.

usage: ./build.sh [options] [--] [extra args passed to cmake]

Creates the build directory.
Sets useful build varialbes and adds options for others.
Runs Cmake with whatever extra args you gave it after \033[0;35m--\033[0;0m.
Attempts a multithreaded build using your generator.

Options:
    \033[0;35m-h | --help\033[0;0m
        Display this message and exit.
    \033[0;35m-c | --clean\033[0;0m
        Remove the build folder before starting the build. (rm)
    \033[0;35m-r | --release\033[0;0m
        Sets the build to make release executables
    \033[0;35m-d | --debug\033[0;0m
        Sets the build to make debug executables
    \033[0;35m--testing\033[0;0m
        Sets the build to make debug executables and builds the unit tests
    \033[0;35m-j N | --nproc N\033[0;0m
        Sets the parallelism flag on the Make tool.
        N: number of threads to use.
        When not specified, attempts to autodetect thread count. (Spare 1)
EOF
        exit 0
        ;;
    *)
        echo "Build Script: unknown option: $1"
        exit 1
        ;;
    esac
    shift
 done


mkdir -p ${BUILDDIR}
pushd ${BUILDDIR}

nprocs=${nprocs:-"$(nproc --ignore=1)"}
if [ -z ${MAKEFLAGS+x} ]; then
  export MAKEFLAGS="-j${nprocs}"
fi


cleanup() {
  exit $SIGNAL;
}

# catch errors during configure/build steps
trap 'SIGNAL=$?;cleanup' ERR
trap 'cleanup' SIGINT

# build with support commands
CMAKE_CONFIGURE_COMMAND=`cat<< EOF
cmake .. -L \
 -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
 ${CMAKE_CONFIGURE_OPTS} \
 $@
EOF
`

CMAKE_BUILD_COMMAND=`cat<< EOF
cmake --build . \
  $CMAKE_BUILD_OPTS
EOF
`

echo "CMake configure being run:"
echo -e "\033[0;96m${CMAKE_CONFIGURE_COMMAND}\033[0;0m"
$CMAKE_CONFIGURE_COMMAND

echo "CMake build being run:"
echo -e "\033[0;96m${CMAKE_BUILD_COMMAND}\033[0;0m"

make_retval=1
trap '' ERR
${CMAKE_BUILD_COMMAND}
build_retval=$?
trap 'SIGNAL=$?;cleanup' ERR

if [ ${build_retval} -eq 0 ]; then
  echo -e "Build \033[0;92mcompleted.\033[0;0m"
else
  echo -e "\033[0;31m !!! Compilation failed. \033[0m"
  exit $make_retval
fi