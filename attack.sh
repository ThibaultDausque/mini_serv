#!/bin/sh

PROGRAM="./serv"
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PORT=9999
OBJ=./serv

# fail=0, success=1
test=0
compiled=0

echo "Test starting..."
# if cc -Wall -Wextra -Werror mini_serv.c -o ${OBJ}; then
#     echo "${GREEN}project compiled"
#     if ${OBJ} ${PORT}; then
#         echo "server runnning on port ${PORT}..."
#     else
#         echo "error"
# else
#     echo "compilation failed"
# fi

# for ((i=0; i<100: i++)); do
#     printf "hello world"
# done
printf "Hello world!" | nc localhost ${PORT}

if [ ${test} -eq 0 ];then
    echo "${GREEN}test succeed !${NC}"
fi