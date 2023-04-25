#!/bin/bash

#this script must be run in the tests folder bc of below path dependency
../build/bin/webserver GetPortNumber/success_pn1 &

webserverPID=$!

number=0
numberOfTests=0
numberOfSucceededTests=0
finalExit=0
fname=IntegrationDiffs/integration_curl_result.txt

# ----------------
# BASIC CURL TEST

curl localhost:80 -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_curl_basic.txt" "$fname"

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
# TODO: NC INCORRECT TEST
# This will work once we have implemented header parsing in Assignment 4.

# fname=IntegrationDiffs/integration_nc_incorrect_result.txt

# echo -e "Nonsense Input" | \
#     nc localhost 80 > "$fname"

# diff -b "IntegrationDiffs/expected_nc_incorrect.txt" "$fname"

# diffExit=$?

# if [[ "$diffExit" -eq 1 ]]
# then 
#     printf "nc incorrect nonsense test: failure\n"
#     (( ++numberOfTests ))
#     finalExit=1
# else
#     printf "nc incorrect nonsense test: success\n"
#     (( ++numberOfTests ))
#     (( ++numberOfSucceededTests ))
# fi


# -----------------



printf "Tests Passed: $numberOfSucceededTests/$numberOfTests\n"

kill $webserverPID

exit $finalExit



#diff btwn expected curl and curl result (IntegrationDiffs/expected_curl_basic.txt)







