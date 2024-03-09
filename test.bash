#!/bin/bash

PORT="$(( 1000 + ( $RANDOM % ( 32768 - 1000 ) ) ))"

./gnaws "$PORT" &
PID="$!"

for x in `seq 1000`; do
	if !(netcat localhost "$PORT" | diff -q response.http -); then
		echo "Netcat test $x failed"
		kill "$PID"
		exit 1
	fi
done

for x in `seq 1000`; do
	if !(curl -s "localhost:$PORT" | diff -q index.html -); then
		echo "Curl test $x failed"
		kill "$PID"
		exit 1
	fi
done

kill "$PID"
