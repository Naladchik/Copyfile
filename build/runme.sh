#!/bin/bash 

./copy_file

if diff file_to_copy.txt copy.txt > /dev/null; then
	echo "file_to_copy.txt and copy.txt are the same"
	rm copy.txt
	echo "copy.txt was deleted"
	echo ""
else
	echo "file_to_copy.txt and copy.txt are different!"
fi

./copy_file

if diff file_to_copy.txt copy.txt > /dev/null; then
	echo "file_to_copy.txt and copy.txt are the same"
	rm copy.txt
	echo "copy.txt was deleted"
	echo ""
else
	echo "file_to_copy.txt and copy.txt are different!"
fi

./copy_file

if diff file_to_copy.txt copy.txt > /dev/null; then
	echo "file_to_copy.txt and copy.txt are the same"
	rm copy.txt
	echo "copy.txt was deleted"
	echo ""
else
	echo "file_to_copy.txt and copy.txt are different!"
fi

