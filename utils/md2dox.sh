#!/bin/bash
#Author: Mattia Nicolella
#copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
#SPDX-License-Identifier: MIT
# Convert a .md file to a .dox file making it ready to be included in the documentation
fname=$1
if [[ $1 == "" ]]; then
    echo "Missing input file name! Exiting."
    exit
fi
cp $fname.md $fname.dox
#Open the doxygen C-style comment
sed -i "1i /**" $fname.dox
#uncomment html comments
sed -i "s/<!--//" $fname.dox
sed -i "s/-->//" $fname.dox
# make sure that doxygen @todo and @bug are not in bullet lists
sed -i "s/\s*-\s*@todo/@todo/" $fname.dox
sed -i "s/\s*-\s*@bug/@bug/" $fname.dox
#cose the doxygen C-style comment
echo " */" >> $fname.dox
