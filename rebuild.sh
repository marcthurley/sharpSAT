#!/bin/bash

# This script is used to rebuild the binary
# for the sampler including transferring the
# latest source code.
BIN_NAME=sharpSAT

printf "Starting the uniform sampler rebuild...\n"
printf "Deleting the \"build\" directory...\n"
rm -rf build &> /dev/null

printf "Running the setup script...\n"
bash setupdev.sh &> /dev/null
cd build/Release &> /dev/null

printf "Making the \"Release\" program...\n"
make &> /dev/null
if [ ! -f $BIN_NAME ]; then
   printf "\033[1;31mMAKE FAILED\033[0m\n"
fi

cd ../.. &> /dev/null
printf "Rebuild completed.\n"
