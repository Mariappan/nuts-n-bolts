#! /bin/bash
function f() {
    name=$1[@]
    b=$2
    a=("${!name}")

    for i in "${a[@]}" ; do
        echo "$i"
    done
    echo "b: $b"
}

declare -a x=("one two" "LAST")
b='even more'

f x "$b"
