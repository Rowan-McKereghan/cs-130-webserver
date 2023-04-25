#!/bin/bash

#this script must be run in the tests folder bc of below path dependency
../build/bin/webserver GetPortNumber/success_pn1 &

webserverPID=$!

number=0
numberOfTests=0
numberOfSucceededTests=0
finalExit=0
fname=IntegrationDiffs/integration_result.txt

while [ -e "$fname" ]; do #ensure no duplicate integration test output files, they will be present in the IntegrationDiffs directory
    printf -v fname '%s-%02d.txt' "IntegrationDiffs/integration_result.txt" "$(( ++number ))"
done
# no test results will be pushed (i added it to the gitignore)
# but we can make test file names specific to tests manually if need be

# ----------------
# BASIC CURL TEST

curl localhost:80 -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_curl_basic.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "basic curl test: failure\n"
    (( ++numberOfTests ))
else
    printf "basic curl test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

finalExit=$finalExit||$diffExit #put this line at the end of every test added so that the exit code is 1 if any test fails

# ----------------
# NC INCORRECT TEST
# This will work once we have implemented header parsing in Assignment 4.

# while [ -e "$fname" ]; do #ensure no duplicate integration test output files
#     printf -v fname '%s-%02d.txt' "IntegrationDiffs/integration_result.txt" "$(( ++number ))"
# done

# echo -e "Nonsense Input" | \
#     nc localhost 80 > "$fname"

# diff -b "IntegrationDiffs/expected_nc_incorrect.txt" "$fname"

# diffExit=$?

# if [[ "$diffExit" -eq 1 ]]
# then 
#     printf "nc incorrect nonsense test: failure\n"
#     (( ++numberOfTests ))
# else
#     printf "nc incorrect nonsense test: success\n"
#     (( ++numberOfTests ))
#     (( ++numberOfSucceededTests ))
# fi

# finalExit=$finalExit||$diffExit

# -----------------



printf "Tests Passed: $numberOfSucceededTests/$numberOfTests\n"

kill $webserverPID

exit $finalExit



#diff btwn expected curl and curl result (IntegrationDiffs/expected_curl_basic.txt)







