CFLAGS=-Wall -Wextra -std=c11 -O2

quantization: find_union.o trie_tree.o parser.o quantization.o
	cc $(CFLAGS) -g -o $@ $^

find_union.o: find_union.c find_union.h
trie_tree.o: trie_tree.c trie_tree.h find_union.h
parser.o: parser.c parser.h
quantization.o: quantization.c parser.h trie_tree.h find_union.h

.o:
	cc $(CFLAGS) -c $<

clean:
	rm quantization *.o

.PHONY: clean all
