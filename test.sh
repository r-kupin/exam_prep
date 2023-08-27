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
    
    # async run with array of args, save output and retrieve process pid
    (./microshell "${words[@]}") > microshell_output.txt & pid=$!
    # wait, while process will reach a pause before the end of execution
    sleep 1

    # count open fd's
    fd_count=$(ls /proc/$pid/fd | wc -l)
    # substract fd's that are open by default
    fd_count=$((fd_count - DEFAULT_FD))
    # print error or success
    if [ "$fd_count" -ne 0 ]; then
        echo -e "${RED}/proc/$pid/fd open file descriptors: KO${NC}"
    else
        echo -e "${GREEN}/proc/$pid/fd open file descriptors: OK${NC}"
    fi

    # perform valgrind check
    valgrind_output=$(valgrind --leak-check=full --show-leak-kinds=all ./microshell "${words[@]}" 2>&1)

    # check the message, that says "no errors"
    if [[ $valgrind_output == *"0 errors from 0 contexts (suppressed: 0 from 0)"* ]]; then
        echo -e "${GREEN}Valgrind memory leaks check: OK${NC}"
    else
        echo -e "${RED}Valgrind memory leaks check: KO${NC}"
        echo "$valgrind_output"
    fi

    # valgrind fd check
    if [[ $valgrind_output == *"Open file descriptor"* ]]; then
        echo -e "${RED}Valgrind open file descriptors: KO${NC}"
    else
        echo -e "${GREEN}Valgrind open file descriptors: OK${NC}"
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

gcc -Wall -Wextra -Werror microshell.c -D TEST=1 -o microshell

run_and_compare '/bin/ls | /usr/bin/grep "shell" ; /bin/echo "done!"'
run_and_compare '/bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls '
run_and_compare '/bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls ; /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls | /bin/ls '
run_and_compare ';'
run_and_compare '; ; '
run_and_compare '; ; /bin/cat microshell.c'
run_and_compare '; ; /bin/ls microshell.c'
run_and_compare '/bin/cat microshell.c ;'
run_and_compare '/bin/cat microshell.c ; ; /bin/echo OK'
run_and_compare '/bin/ls ewqew | /usr/bin/grep micro | /bin/cat -n ; /bin/echo dernier ; /bin/echo'
run_and_compare '/bin/ls | /usr/bin/grep microshell | /usr/bin/grep micro | /usr/bin/grep shell | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep micro | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell | /usr/bin/grep shell'

echo "All tests completed"

