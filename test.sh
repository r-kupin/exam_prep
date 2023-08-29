#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'
DEFAULT_FD=6

run_and_compare() {
    # retrieve the argument
    command="$1"
    echo "Running: $command"

    touch system_output.txt microshell_output.txt

    # eval execures given string as if it were run in shell and saves output
    # if command is incorrect - eval fails
    (eval "$command" > system_output.txt)
    eval_failed=$?

    # Brake down command string to array of words
    IFS=' ' read -ra words <<< "$command"

    # remove excessive quotations in the beginning and end of the word, if any
    for (( i = 0; i < ${#words[@]}; i++ )); do
        word="${words[i]}"
        if [[ $word == "\""* && $word == *"\"" ]]; then
        words[i]=${word:1:-1}  
    fi
        done
    
    ./microshell "${words[@]}" > microshell_output.txt & pid=$!
   
    # perform valgrind check
    valgrind_output=$(valgrind --leak-check=full --show-leak-kinds=all ./microshell "${words[@]}" 2>&1)
    # check the message, that says "no errors"
    if [[ $valgrind_output == *"0 errors from 0 contexts (suppressed: 0 from 0)"* ]]; then
        echo -e "${GREEN}Valgrind memory leaks check: OK${NC}"
    else
        echo -e "${RED}Valgrind memory leaks check: KO${NC}"
        echo "$valgrind_output"
    fi

    # perform valgrind check
    valgrind_output=$(valgrind -q --track-fds=all ./microshell "${words[@]}" 2>&1)
    # valgrind fd check
    if [[ $valgrind_output == *"FILE DESCRIPTORS: 3 open (3 std) at exit"* ]]; then
        echo -e "${GREEN}Valgrind open file descriptors: OK${NC}"
    else
        echo -e "${RED}Valgrind open file descriptors: KO${NC}"
        echo "$valgrind_output"
    fi

    # if eval commend succedded
    if [ "$eval_failed" -eq 0 ]; then
    # compare the output
        if diff -q system_output.txt microshell_output.txt >/dev/null; then
            echo -e "${GREEN}Output compared: OK${NC}"
        else
            echo -e "${RED}Output compared: KO${NC}"
            diff system_output.txt microshell_output.txt
        fi
    else
    # if not - check yourself
        echo -e "${BLUE}eval failed, nothing to compare${NC}"
        cat microshell_output.txt
    fi

    rm -f system_output.txt microshell_output.txt
    echo "-------------------"
}

# usage: bash test.sh microshell-filename-dot-c
gcc -Wall -Wextra -Werror "$1" -o microshell

# compare output with shell
run_and_compare '/bin/ls | /usr/bin/grep "shell" ; /bin/echo "done!"'
run_and_compare '/bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls '
run_and_compare '/bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls '
run_and_compare ';'
run_and_compare '; ; '
run_and_compare '; ; /bin/cat microshell.c | /usr/bin/grep "NULL"'
run_and_compare '; ; /bin/ls microshell.c | /usr/bin/grep "NULL"'
run_and_compare '/bin/cat microshell.c | /usr/bin/grep "NULL" ;'
run_and_compare '/bin/cat microshell.c | /usr/bin/grep "NULL" ; ; /bin/echo OK'
run_and_compare '/bin/ls ewqew | /usr/bin/grep micro | /bin/cat -n ; /bin/echo dernier ; /bin/echo'
run_and_compare '/bin/ls | /usr/bin/grep microshell | /usr/bin/grep micro | /usr/bin/grep shell | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell'

gcc -Wall -Wextra -Werror -g -fsanitize=address "$1" -o microshell

cd_compare() {
    diff_result=$(diff <(echo "$1") <(echo "$2"))
    if [ -z "$diff_result"]; then
        echo -e "${GREEN}diff: OK${NC}"
    else
        echo -e "${RED}diff: $diff_result${NC}"
    fi
}

current_address=$(pwd) 
dir_addr="test"
mkdir "$dir_addr"
example_addr="${current_address}/${dir_addr}"

echo "Running: cd $dir_addr"
after_cd_up_address=$(./microshell cd $dir_addr ";" /bin/pwd)
rm -rf test
cd_compare "$after_cd_up_address" "$example_addr"

echo "Running: cd $dir_addr $dir_addr"
after_cd_up_address=$(./microshell cd $dir_addr $dir_addr 2>&1)
cd_compare "$after_cd_up_address" "error: cd: bad arguments"

echo "Running: cd"
after_cd_up_address=$(./microshell cd 2>&1)
cd_compare "$after_cd_up_address" "error: cd: bad arguments"

dir_addr="..."
echo "Running: cd $dir_addr"
after_cd_up_address=$(./microshell cd $dir_addr 2>&1)
cd_compare "$after_cd_up_address" "error: cd: cannot change directory to $dir_addr"

dir_addr="blah"
echo "Running: cd $dir_addr"
after_cd_up_address=$(./microshell cd $dir_addr 2>&1)
cd_compare "$after_cd_up_address" "error: cd: cannot change directory to $dir_addr"

echo "All tests completed"

