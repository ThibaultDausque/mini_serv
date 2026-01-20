#!/bin/sh

PROGRAM="./serv"
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

PORT=9999
OBJ=./serv

kill $(lsof -ti:$PORT) > /dev/null 2>&1

# fail=0, success=1
test=0
compiled=0

FLAG="-Wall -Wextra -Werror -g"

echo "Test starting..."
if cc ${FLAG} mini_serv.c -o ${OBJ}; then
    echo "${GREEN}project compiled"
else
    echo "compilation failed"
    exit 1
fi

touch server.log
${OBJ} ${PORT} > server.log 2>&1 &
PID=$!
if [ $? -eq 0 ]; then
    echo "server runnning on port ${PORT}..."
else
    echo "${RED}Fatal error${NC}"
fi

nc localhost ${PORT} &
NCPID=$!
if [ $? -eq 0 ]; then
    echo "${GREEN}test 1: OK${NC}"
else
    echo "${RED}test 1: KO${NC}"
fi

cat server.log

if ps -p $PID > /dev/null; then
    kill $PID
fi

if ps -p $NCPID > /dev/null; then
    kill $NCPID
fi

if [ $? -eq 0 ]; then
    echo "done"
fi