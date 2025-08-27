#!/bin/bash 

./copy_file

if diff source.txt destination.txt > /dev/null; then
	echo "source.txt and destination.txt are the same"
	rm destination.txt
	echo "destination.txt was deleted"
else
	echo "source.txt and destination.txt are are different!"
fi

./copy_file

if diff source.txt destination.txt > /dev/null; then
	echo "source.txt and destination.txt are the same"
	rm destination.txt
	echo "destination.txt was deleted"
else
	echo "source.txt and destination.txt are are different!"
fi

./copy_file

if diff source.txt destination.txt > /dev/null; then
	echo "source.txt and destination.txt are the same"
	rm destination.txt
	echo "destination.txt was deleted"
else
	echo "source.txt and destination.txt are are different!"
fi
