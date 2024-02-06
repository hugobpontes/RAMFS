#!/bin/bash
set -e

BuildFolder="./build/"
ExecName="RAMFS_tests"

echo "--> Generating"
cmake -B${BuildFolder}
echo "--> Building"
cmake --build ${BuildFolder} 
echo "--> Running"
${BuildFolder}/${ExecName} -c -v
echo "--> Finished"