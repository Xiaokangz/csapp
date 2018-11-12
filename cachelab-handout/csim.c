/*
 * csim.c - A cache simulator
 *
 * Author: Xiaokang Zhang
 * Email: xiaokangz00@gmail.com
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define ULLEN sizeof(unsigned long) * 8

/* A struct that stores valid bit, tag and LRU counter. */
typedef struct 
{
	char valid_bit;
	unsigned long tag;
	int time_stamps;
} cache_line;

/* initialize the cache simulator */
void init_cache(cache_line** cache_arr, int num_sets, int E)
{
	int i, j;
	for (i = 0; i < num_sets; i++){
		for (j = 0; j < E; j++) {
			cache_arr[i][j].valid_bit = 0;
			cache_arr[i][j].time_stamps = 0;
		}
	}
}

/* find the line in cache where the new address should take */
int find_line(cache_line** cache_arr, int index, int E, int tag) 
{
	int line = -1;
	int i;
	int max=0, max_index = -1;
	int blank_index = -1;
	for (i = 0; i < E; i++) {
		if (cache_arr[index][i].valid_bit) {
			if (cache_arr[index][i].tag == tag) {
				line = i;
			}
			if (max_index == -1 || cache_arr[index][i].time_stamps > max) {
				max_index = i;
				max = cache_arr[index][i].time_stamps;
			}
		} else {
			blank_index = i;
		}
	}
	if (line == -1) {
		if (blank_index != -1) {
			line = blank_index;
		} else {
			line = max_index;
		}
	}
	for (i = 0; i < E; i++) {
		if (i != line && cache_arr[index][i].valid_bit) {
			cache_arr[index][i].time_stamps += 1;
		}
	}
	return line;
}

/* return different flag depending on hit, miss and eviction */
int situation(cache_line** cache_arr, int index, int line, int tag)
{
	int flag;
	if (cache_arr[index][line].valid_bit) {
		if (cache_arr[index][line].tag == tag) {
			flag = 1;
		} else {
			flag = -1;
		}
	} else {
		flag = 0;
	}
	cache_arr[index][line].tag = tag;
	cache_arr[index][line].valid_bit = 1;
	cache_arr[index][line].time_stamps = 0;
	return flag;
}

/* print one operation info */
void print_info(char* operation, unsigned long address, int size, int hit, int miss, int eviction)
{
	printf("%s %lx,%d ", operation, address, size);
	if (miss > 0) {
		printf("miss ");
	}
	if (hit > 0) {
		printf("hit ");
	}
	if (eviction > 0) {
		printf("eviction");
	}
	printf("\n");
}

int main(int argc, char* argv[])
{
	// read arguments from command line
	int opt, s, E, b;
	int vflag = 0;
	int num_sets;
	int hits = 0, misses = 0, evictions = 0;
	cache_line **cache_arr;
	char* trace_file;
	int i;
	char operation[5];
	unsigned long address;
	int size;
	int tag, index;
	unsigned long mask1 = 0, mask2 = 0;
	int line, flag;
	FILE* trace_file_stream;
	while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (opt) {
			case 'h':
				break;
			case 'v':
				vflag = 1;
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				trace_file = optarg;
				break;
			default:
				printf("wrong argument\n");
				break;
		}
	}
	// create a cache simulater and initialize it
	num_sets = 1 << s;
	cache_arr = (cache_line **) malloc(sizeof(cache_line *) * num_sets);
	for (i = 0; i < num_sets; i++) {
		cache_arr[i] = (cache_line *) malloc(sizeof(cache_line) * E);
	}
	init_cache(cache_arr, num_sets, E);
	// simulate the cache
	mask1 = (~ mask1) >> (ULLEN - s);
	mask2 = (~ mask2) >> (b + s);
	trace_file_stream = fopen(trace_file, "r");
	while (fscanf(trace_file_stream, "%s%lx,%d", operation, &address, &size) != EOF) {
		index = (address >> b) & mask1;
		tag = (address >> (b + s)) & mask2;
		if (strcmp(operation, "L") == 0 || strcmp(operation,"S") == 0) {
			line = find_line(cache_arr, index, E, tag);
			flag = situation(cache_arr, index, line, tag);
			if (flag == 1) {
				hits += 1;
				if (vflag) {
					print_info(operation, address, size, 1, 0, 0);
				}
			} else if (flag == 0) {
				misses += 1;
				if (vflag) {
					print_info(operation, address, size, 0, 1, 0);
				}
			} else if (flag == -1) {
				misses += 1;
				evictions += 1;
				if (vflag) {
					print_info(operation, address, size, 0, 1, 1);
				}
			}
		} else if (strcmp(operation, "M") == 0) {
			line = find_line(cache_arr, index, E, tag);
			flag = situation(cache_arr, index, line, tag);
			if (flag == 1) {
				hits += 2;
				if (vflag) {
					print_info(operation, address, size, 2, 0, 0);
				}
			} else if (flag == 0) {
				misses += 1;
				hits += 1;
				if (vflag) {
					print_info(operation, address, size, 1, 1, 0);
				}
			} else if (flag == -1) {
				misses += 1;
				hits += 1;
				evictions += 1;
				if (vflag) {
					print_info(operation, address, size, 1, 1, 1);
				}
			}
		}
	} 
	fclose(trace_file_stream);
	// free the cache simulater 
	for (i = 0; i < num_sets; i++) {
		free(cache_arr[i]);
	}
	free(cache_arr);
    printSummary(hits, misses, evictions);
    return 0;
}
