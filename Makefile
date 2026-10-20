all: elfinfo minidbg target
elfinfo: elfinfo.c
	gcc -Wall -O2 -o elfinfo elfinfo.c
minidbg: minidbg.c
	gcc -Wall -O2 -o minidbg minidbg.c
target: target.c
	gcc -O0 -no-pie -o target target.c
clean:
	rm -f elfinfo minidbg target
