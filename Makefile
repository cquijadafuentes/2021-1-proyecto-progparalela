CPP=gcc
OBJECTS=basic.o bitrankw32int.o kTree.o Entry.o NodeQueue.o Queue.o misBits.o \
	k2tree_operations.o adylist.o adylist_operations.o k2tree_operations_parallel.o

BINS=build_tree use_tree test_tree rebuild_tree invrebuild_tree revtest_tree \
	rebuildCheck_tree fulldecompress_tree \
	k2tree_setop_intersection adylist_setop_intersection \
	k2tree_setop_intersection_parallel
	
#CPPFLAGS=-Wall -g3 
CPPFLAGS=-Wall -O9 -g -DNDEBUG 
DEST=.

%.o: %.c
	$(CPP) $(CPPFLAGS) -c $< -o $@

all: bin

bin: $(OBJECTS) $(BINS)

build_tree:
	gcc $(CPPFLAGS) -o $(DEST)/build_tree buildk2tree.c $(OBJECTS) -lm

use_tree:
	gcc $(CPPFLAGS) -o $(DEST)/use_tree use.c $(OBJECTS) -lm

test_tree:
	gcc $(CPPFLAGS) -o $(DEST)/test_tree testSpeed.c  $(OBJECTS) -lm

revtest_tree:
	gcc $(CPPFLAGS) -o $(DEST)/revtest_tree testSpeedPred.c  $(OBJECTS) -lm
	
rebuild_tree:
	gcc $(CPPFLAGS) -o $(DEST)/rebuild_tree rebuild.c  $(OBJECTS) -lm

rebuildCheck_tree:
	gcc $(CPPFLAGS) -o $(DEST)/rebuildCheck_tree checkrebuild.c $(OBJECTS) -lm

invrebuild_tree:
	gcc $(CPPFLAGS) -o $(DEST)/invrebuild_tree reverserebuild.c $(OBJECTS) -lm
	
fulldecompress_tree:
	gcc $(CPPFLAGS) -o $(DEST)/fulldecompress_tree fulldecompression.c $(OBJECTS) -lm

k2tree_setop_intersection:
	gcc $(CPPFLAGS) -o $(DEST)/k2tree_setop_intersection k2tree_setop_intersection.c $(OBJECTS) -lm

adylist_setop_intersection:
	gcc $(CPPFLAGS) -o $(DEST)/adylist_setop_intersection adylist_setop_intersection.c $(OBJECTS) -lm

k2tree_setop_intersection_parallel:
	gcc $(CPPFLAGS) -o $(DEST)/k2tree_setop_intersection_parallel k2tree_setop_intersection_parallel.c $(OBJECTS) -lm
		
clean:
	rm -f $(OBJECTS) $(BINS)
	cd $(DEST); rm -f *.a $(BINS)

