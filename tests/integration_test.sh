#!/bin/bash

#running in root directory to match relative paths for config file in production
cd ..
# clear output from previous tests
rm tests/IntegrationDiffs/integration_result*

if [ -e "./build/bin/webserver" ]
then
    ./build/bin/webserver ./configs/dev_config &
else
    ./build_coverage/bin/webserver configs/dev_config &
fi

webserverPID=$!

number=0
numberOfTests=0
numberOfSucceededTests=0
finalExit=0
fname=tests/IntegrationDiffs/integration_result_curl_basic.txt

# ----------------
# BASIC CURL TEST
# Should return nothing
curl localhost:80 -s -S -o "$fname"

diff -b  -I '^Date*' "tests/IntegrationDiffs/expected_curl_basic.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "basic curl test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "basic curl test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi


# ----------------
# NC INCORRECT TEST

fname=tests/IntegrationDiffs/integration_result_nc_incorrect_result.txt

echo -e "Nonsense Input" | \
    nc localhost 80 > "$fname"

diff -b -I '^Date*' "tests/IntegrationDiffs/expected_nc_incorrect.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "nc incorrect nonsense test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "nc incorrect nonsense test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

# -----------------

# ----------------
# TXT FILE TRANSFER TEST

fname=tests/IntegrationDiffs/integration_result_txt_file_transfer.txt

curl localhost:80/static/sample.txt -s -S -o "$fname"

diff -b "tests/IntegrationDiffs/test.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "txt file transfer test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "txt file transfer test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

# -----------------

# ----------------
# JPEG FILE TRANSFER TEST

fname=tests/IntegrationDiffs/integration_result_jpeg_file_transfer.jpeg

curl localhost:80/static/sample.jpeg -s -S -o "$fname"

diff -b "tests/IntegrationDiffs/test.jpeg" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "txt file transfer test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "txt file transfer test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

# -----------------

# ----------------
# PDF FILE TRANSFER TEST

fname=tests/IntegrationDiffs/integration_result_pdf_file_transfer.pdf

curl localhost:80/static/sample.pdf -s -S -o "$fname"

diff -b "tests/IntegrationDiffs/test.pdf" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "pdf file transfer test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "pdf file transfer test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

# -----------------

# ----------------
# ZIP FILE TRANSFER TEST

fname=tests/IntegrationDiffs/integration_result_zip_file_transfer.zip

curl localhost:80/static/sample.zip -s -S -o "$fname"

diff -b "tests/IntegrationDiffs/test.zip" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "zip file transfer test: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "zip file transfer test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi



# -----------------

# -----------------

#POST
# CRUD Handler Valid Create action
# Creates an entity and makes sure it's present


SERVER_IP=localhost
SERVER_PORT=80
TIMEOUT=0.5

#rm -rf ../crud/test/1
rm -rf ../ctrl-c-ctrl-v/crud/integration_test
#rm -rf ../api/test
#rm -rf entitydata??
sleep 1.0

# Curl server and create
timeout $TIMEOUT curl -s -i -X POST -H "Host:" -H "User-Agent:" $SERVER_IP:$SERVER_PORT/api/integration_test -d '{"a":111}'

# Check if file exists or not
FILE=../ctrl-c-ctrl-v/crud/integration_test/1

test -f "$FILE";
RESULT=$?

if [ $RESULT -eq 0 ];
then
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
    echo "$FILE exists."
else 
    echo "$FILE does NOT exist."
    (( ++numberOfTests ))
    finalExit=1
fi


# -----------------

# -----------------

#GET
# CRUD Handler Valid Retrieve action
# Retrieves an entity and makes sure it's correct



# Curl server and get list
timeout $TIMEOUT curl -s -i -X GET -H "Host:" -H "User-Agent:" $SERVER_IP:$SERVER_PORT/api/integration_test/1 > integration_test_GET1_result

# assign result to a variable

fname=integration_test_GET1_result


#ignore date as that is a temporary metric and compare the rest to confirm info is accurate
diff -b -I '^Date*' "tests/IntegrationDiffs/integration_test_GET1_exp" "$fname"

RESULT=$?


if [[ "$RESULT" -eq 1 ]]
then 
    printf "GET: failure\n"
    (( ++numberOfTests ))
    finalExit=1
else
    printf "GET: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi





# -----------------

# -----------------



#PUT
# CRUD Handler Valid Update action

# Curl server and update entity
timeout $TIMEOUT curl -s -i -d '{"a":123}' -X PUT -H "Host:" -H "User-Agent:" $SERVER_IP:$SERVER_PORT/api/integration_test/1

# Curl server and get updated entity
timeout $TIMEOUT curl -s -i -X GET -H "Host:" -H "User-Agent:" $SERVER_IP:$SERVER_PORT/api/integration_test/1 > integration_test_PUT_result

# Check if file exists or not
RESULT=diff tests/IntegrationDiffs/integration_test_PUT_exp integration_test_PUT_result | grep "^<" | wc -l



fname=tests/IntegrationDiffs/integration_test_PUT_exp

DIFFLINE= diff "integration_test_PUT_result" "$fname" | grep "^<" 

DIFFWORD= $DIFFLINE | head -n1 | cut -d " " -f1
#isolates the first word of the different line to check if it's the Date line

rm integration_test_PUT_result

if ([ $RESULT -eq 1 ] && [ $DIFFWORD -eq Date:]);
then
    #TEST_PUT_SUCCESS=1
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
    echo "Test PUT Success"
elif [ $RESULT -eq 0 ];
then
    #TEST_GET_SUCCESS=1
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
    echo "Test PUT Success"
else
    #TEST_PUT_SUCCESS=0
    echo "Test PUT Failure"
    (( ++numberOfTests ))
    finalExit=1
fi


# -----------------

# -----------------


#DELETE
# CRUD Handler Valid remove action


# Curl server and delete
timeout $TIMEOUT curl -s -i -X DELETE -H "Host:" -H "User-Agent:" $SERVER_IP:$SERVER_PORT/api/integration_test/1

# Check if file exists or not
FILE=.../ctrl-c-ctrl-v/crud/integration_test/1

test -f "$FILE";
RESULT=$?

if [ $RESULT -ne 0 ];
then
    #TEST_DELETE_SUCCESS=1
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
    echo "$FILE was deleted."
else 
    #TEST_DELETE_SUCCESS=0
    echo "$FILE was not deleted."
    (( ++numberOfTests ))
    finalExit=1
fi


# -----------------

# -----------------


printf "Tests Passed: $numberOfSucceededTests/$numberOfTests\n"

kill $webserverPID

exit $finalExit



#diff btwn expected curl and curl result (tests/IntegrationDiffs/expected_curl_basic.txt)




