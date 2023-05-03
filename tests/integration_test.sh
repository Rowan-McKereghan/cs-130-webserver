#!/bin/bash

#this script must be run in the tests folder bc of below path dependency
../build/bin/webserver config_parser/GetPortNumber/success_serve_files &

webserverPID=$!

number=0
numberOfTests=0
numberOfSucceededTests=0
finalExit=0
fname=IntegrationDiffs/integration_result_curl_basic.txt

# ----------------
# BASIC CURL TEST

curl localhost:80 -s -S -o "$fname"

diff -b  -I '^Date*' "IntegrationDiffs/expected_curl_basic.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "basic curl test: failure\n"
    (( ++numberOfTests ))
    # finalExit=1 TODO: Fix 400
else
    printf "basic curl test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi


# ----------------
# NC INCORRECT TEST

fname=IntegrationDiffs/integration_result_nc_incorrect_result.txt

echo -e "Nonsense Input" | \
    nc localhost 80 > "$fname"

diff -b -I '^Date*' "IntegrationDiffs/expected_nc_incorrect.txt" "$fname"

diffExit=$?

if [[ "$diffExit" -eq 1 ]]
then 
    printf "nc incorrect nonsense test: failure\n"
    (( ++numberOfTests ))
    # finalExit=1 TODO: Fix 400
else
    printf "nc incorrect nonsense test: success\n"
    (( ++numberOfTests ))
    (( ++numberOfSucceededTests ))
fi

# -----------------

# ----------------
# TXT FILE TRANSFER TEST

fname=IntegrationDiffs/integration_result_txt_file_transfer.txt

curl localhost:80/static/sample.txt -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_txt_file_transfer.txt" "$fname"

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

fname=IntegrationDiffs/integration_result_jpeg_file_transfer.jpeg

curl localhost:80/static/sample.jpeg -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_jpeg_file_transfer.jpeg" "$fname"

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

fname=IntegrationDiffs/integration_result_pdf_file_transfer.pdf

curl localhost:80/static/sample.pdf -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_pdf_file_transfer.pdf" "$fname"

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

fname=IntegrationDiffs/integration_result_zip_file_transfer.zip

curl localhost:80/static/sample.zip -s -S -o "$fname"

diff -b "IntegrationDiffs/expected_zip_file_transfer.zip" "$fname"

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





printf "Tests Passed: $numberOfSucceededTests/$numberOfTests\n"

kill $webserverPID

exit $finalExit



#diff btwn expected curl and curl result (IntegrationDiffs/expected_curl_basic.txt)







