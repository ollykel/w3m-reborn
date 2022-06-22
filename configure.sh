#!/bin/bash -x

# Generates a Makefile

# Misc. constants
TAB=$'\t'
CONFIG_ARGS=($@)

# Standard options
export BIN=w3m
export TEST_DIR=tests
export CPP=g++
export LD="${CPP}"
export PREFIX="/usr/local"

# parse options from args
while [[ $# -gt 0 ]]; do
    IFS='=' read key val <<< "$1"
    case "${key}" in
        --with-bin)
            if [[ ${val} ]]; then
                export BIN="${val}"
            else
                export BIN="$2"
                shift
            fi
            ;;
        --with-cpp)
            if [[ ${val} ]]; then
                export CPP="${val}"
            else
                export CPP="$2"
                shift
            fi
            ;;
        --with-ld)
            if [[ ${val} ]]; then
                export LD="${val}"
            else
                export LD="$2"
                shift
            fi
            ;;
        --with-prefix)
            if [[ ${val} ]]; then
                export PREFIX="${val}"
            else
                export PREFIX="$2"
                shift
            fi
            ;;
    esac
    shift
done
# end arg parsing

# util functions
get_deps()
{
    local target_file="$1"
    local -A file_set
    local -a file_queue

    IFS=$'\n\r' local files=($(
        grep -e "^\s*#include\s\+\"\S\+\"\s*\$" < "${target_file}" | \
        sed -re "s@^\s*#include\s+\"(\S+)\"\s*\$@\1@g" | \
        sed -re "s@^(\.\./)*@@"))

    file_queue+=(${files[@]})
    for file in ${files[@]}; do
        file_set[${file}]=1
    done

    while [[ ${file_queue} ]]; do
        local target="${file_queue[0]}"

        [[ ! -f "${target}" ]] && continue

        # output the value here
        echo "${target}"

        IFS=$'\n\r' local deps=($(
            grep -e "^\s*#include\s\+\"\S\+\"\s*\$" < "${target}" | \
            sed -re "s@^\s*#include\s+\"(\S+)\"\s*\$@\1@g" | \
            sed -re "s@^(\.\./)*@@"))
        
        for dep in ${deps[@]}; do
            if [[ ! ${file_set[${dep}]} ]]; then
                file_queue+=(${dep})
            fi
            file_set[${dep}]=1
        done

        file_queue=(${file_queue[@]:1})
    done
}
# end get_deps

format_obj_build_deps()
{
    local file_base="$1"

    if [[ ! "${file_base}" || ! -f "${file_base}.cpp" ]]; then
        return 1
    fi

    IFS=$'\n\r' local -a deps=($(get_deps "${file_base}.cpp"))

    cat << _EOF_
${file_base}.o : ${file_base}.cpp ${deps[@]}

_EOF_
}
# end format_obj_build_deps

format_test_build_deps()
{
    local test_base="$1"

    if [[ ! "${test_base}" || ! -f "${test_base}.cpp" ]]; then
        return 1
    fi

    local -a obj_files=("${test_base}.o")

    IFS=$'\n\r' local -a deps=($(get_deps "${test_base}.cpp"))

    for dep in ${deps[@]}; do
        dep_base="$(sed -e "s@\.hpp\$@@" <<< "${dep}")"
        if [[ -f "${dep_base}.cpp" ]]; then
            obj_files+=("${dep_base}.o")
        fi
    done
    
    cat << _EOF_
#-- ${test_base}.out ------------------------------------------------------
${test_base}.out : ${obj_files[@]}

${test_base}.o : ${test_base}.cpp ${deps[@]}

_EOF_
}
# end format_test_build_deps

# lists of files
IFS=$'\n\r' base_cpp_files=($(find -maxdepth 1 -type f -name "*.cpp" | \
    cut -c3- | sed -e "s/\.cpp$//"))
IFS=$'\n\r' base_obj_files=($(for base in ${base_cpp_files[@]}; do
    echo "${base}.o"
done))
IFS=$'\n\r' test_cpp_files=($(find ${TEST_DIR}/ -maxdepth 1 \
    -type f -name "*.cpp" |  sed -e "s/\.cpp$//"))
IFS=$'\n\r' test_obj_files=($(for base in ${test_cpp_files[@]}; do
    echo "${base}.o"
done))
IFS=$'\n\r' test_out_files=($(for base in ${test_cpp_files[@]}; do
    echo "${base}.out"
done))

# XXX START OF MAKEFILE GENERATOR
{
# Generally non-changing elements
cat << _EOF_
TEST_DIR=${TEST_DIR}
CPP=${CPP}
LD=${LD}

ifeq (\$(PREFIX),)
PREFIX=${PREFIX}
endif

ifeq (\$(BIN),)
BIN=${BIN}
endif

%.out : %.o
${TAB}$(if [[ "${LD}" = "${CPP}" ]]; then
    echo "\$(LD) -o \$@ \$^"
else
	echo "\$(LD) ${LDFLAGS} -o \$@ \$^"
fi)

# === ALL =================================================================
#
# =========================================================================
all : main.out tests

# === CLEAN ===============================================================
#
# =========================================================================
clean:
${TAB}\$(RM) main.out
${TAB}\$(RM) ${base_obj_files[@]}
${TAB}\$(RM) ${test_obj_files[@]}
${TAB}\$(RM) ${test_out_files[@]}

# === RECONFIGURE =========================================================
# 
# Rebuilds the Makefile using configure.sh. Run this after
# creating/deleting any .cpp or .hpp files.
# 
# NOTE: this will run ./configure.sh with the arguments passed to it when
# building this Makefile. If you would like to reconfigure with different
# options, run ./configure.sh from scratch.
#
# =========================================================================
reconfigure : configure.sh
${TAB}./configure.sh ${CONFIG_ARGS[@]}

# === DIST CLEAN ==========================================================
#
# =========================================================================
distclean : clean
${TAB}\$(RM) Makefile

# === MAIN BINARY =========================================================
#
# =========================================================================
main.out : ${base_obj_files[@]}
${TAB}$(if [[ "${LD}" = "${CPP}" ]]; then
	echo "\$(LD) -o \$@ \$^"
else
	echo "\$(LD) ${LDFLAGS} -o \$@ \$^"
fi)

# === INSTALL =============================================================
#
# =========================================================================
install : main.out
${TAB}install main.out \$(PREFIX)/bin/\$(BIN)

_EOF_

# Generate object file dependencies
cat << _EOF_
# === OBJECT FILES ========================================================
#
# =========================================================================

_EOF_

for file_base in ${base_cpp_files[@]}; do
    format_obj_build_deps "${file_base}"
done

# Generate test dependencies
cat << _EOF_
# === TESTS ===============================================================
#
# =========================================================================

tests : ${test_out_files[@]}

_EOF_

for file_base in ${test_cpp_files[@]}; do
    format_test_build_deps "${file_base}"
done
} > ./Makefile
# XXX END OF MAKEFILE GENERATOR