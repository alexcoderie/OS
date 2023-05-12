if ! [ $# -eq 2 ]
then
	echo "Not the right number of arguments"
	exit 1
fi

if ! [ -f "$1" ] || ! [ -f "$2" ]
then
	echo "Not a file"
	exit 2
fi

if ! [[ $1 == *.c ]]
then
	echo "Not a C file"
	exit 3
fi

gcc -Wall -o prog "$1" 2> "$2"

count_errors=0
count_warnings=0

count_errors=$(grep -c error "$2")
count_warnings=$(grep -c warning "$2")
echo "Errors: $count_errors Warnings: $count_warnings"