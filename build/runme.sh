#!/bin/bash 

for a in 1 2 3 4 5 6 7 8 9 10
do
./copy_file

if diff source.txt destination.txt > /dev/null; then
	rm destination.txt
else
	echo "source.txt and destination.txt are are different!"
fi
done
