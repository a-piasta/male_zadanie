for f in "$2"/*.in; do
	echo -n "Test ${f#$2/} "
	if diff "${f%in}out" <($1 < "$f" 2>/dev/null) >/dev/null && diff "${f%in}err" <($1 < "$f" 2>&1 >/dev/null) >/dev/null; then
		if (($?==0)); then echo "OK";
		else echo "ERROR: exitcode $?";
		fi
	else echo "WRONG ANSWER";
	fi
	valgrind --leak-check=full --main-stacksize=50000000 --error-exitcode=2 $1 < "$f" >/dev/null 2>/dev/null
	if (($?==2)); then echo "Valgrind check: memleak";
	else echo "Valgrind check: OK";
	fi
done
