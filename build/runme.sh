#!/bin/bash 

for a in 1 2 3 4 5 6 7 8 9 10
do
./copy_file

if diff input.txt output.txt > /dev/null; then
	rm output.txt
else
	echo "input.txt and output.txt are different!"
fi
done
