WARNINGS = -Wall -Warray-bounds=2 -Wcast-align=strict -Wcast-qual -Wconversion -Wno-sign-conversion -Wdangling-else -Wdate-time -Wdouble-promotion -Wextra -Wfloat-conversion -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wformat=2 -Winit-self -Wjump-misses-init -Wlogical-op -Wmissing-include-dirs -Wnested-externs -Wnull-dereference -Wpacked -Wpedantic -Wredundant-decls -Wshadow -Wshift-negative-value -Wshift-overflow=2 -Wstrict-aliasing -Wstrict-overflow=2 -Wstrict-prototypes -Wstringop-overflow=4 -Wstringop-truncation -Wswitch-default -Wswitch-enum -Wuninitialized -Wunsafe-loop-optimizations -Wunused -Wuse-after-free=3 -Wwrite-strings -fanalyzer -fmax-errors=2 -pedantic-errors

CFLAGS = -std=c99 $(WARNINGS)

.PHONY: default
default: test

.PHONY: run
run: gnaws
	./$<

.PHONY: test
test: index.html gnaws
	./gnaws &
	curl -s localhost:8080 | diff -q $< -
	curl -s localhost:8080 | diff -q $< -
	curl -s localhost:8080 | diff -q $< -
	curl -s localhost:8080 | diff -q $< -
	curl -s localhost:8080 | diff -q $< -
	pkill gnaws

gnaws: gnaws.o response.o

gnaws.o: gnaws.c

response.o: response.http
	ld -z noexecstack -r -b binary -o $@ $<

response.http: index.html make_response.sh
	./make_response.sh $< > $@

.PHONY: clean
clean:
	rm -f *.o
	rm -f gnaws response.http
