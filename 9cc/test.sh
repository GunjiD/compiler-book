#!/bin/bash
try() {
	expected="$1"
	input="$2"
	
	./9cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => expected expected, but got $actual"
		exit 1
	fi
}

try 0 '0;'
try 42 '42;'
try 6 '6;'
try 41 ' 12 + 34 - 5 ;'
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 5 '-15+20;'
try 20 '+25-5;'
try 1 '10 == 10;'
try 0 '10 == 11;'
try 1 '10 != 11;'
try 0 '10 != 10;'
try 1 '1 < 3;'
try 0 '3 < 1;'
try 1 '3 <= 3;'
try 1 '20 <= 30;'
try 0 '30 <= 20;'
try 1 '3 > 1;'
try 0 '1 > 3;'
try 1 '3 >= 3;'
try 1 '30 >= 20;'
try 0 '20 >= 30;'
try 1 'a=6; b=6; a==b;'

echo OK
