#!/bin/bash

bold=$(tput bold)
normal=$(tput sgr0)
MUZEC=muzec

run_test_in_directory() {
    RUN_DIR=$1
    pushd .
    cd $RUN_DIR
    TOTAL_TESTS=$(ls | wc -l)
    SUCCESSES=0
    for d in */
    do
        TRIMMED=$(echo $d | sed 's:/*$::')
        cd $d
        $MUZEC -o $TRIMMED $TRIMMED.mz
        if [ $? -eq 0 ]
        then
            ./$TRIMMED > result.txt
            diff result.txt expected.txt
            if [ $? -eq 0 ]
            then
                SUCCESSES=$((SUCCESSES+1))
            else
                echo "Failed test ${bold}$(basename $(pwd))${normal} in $RUN_DIR tests"
            fi
            rm $TRIMMED result.txt a.o
        fi
        cd ..
    done
    echo "Passed" $SUCCESSES "out of" $TOTAL_TESTS "tests in" $RUN_DIR
    popd
}

cd test

run_test_in_directory success
# run_test_in_directory failure
