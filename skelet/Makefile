CC = gcc
CFLAGS = -fPIC -Wall
LDFLAGS =

.PHONY: build
build: libscheduler.so

libscheduler.so: planner.o list.o
	$(CC) $(LDFLAGS) -shared -o $@ $^

planner.o: planner/so_scheduler.c util/so_scheduler.h
	$(CC) $(CFLAGS) -o $@ -c $<

list.o: planner/so_list.c util/so_list.h
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	-rm -f libscheduler.o planner.o list.o