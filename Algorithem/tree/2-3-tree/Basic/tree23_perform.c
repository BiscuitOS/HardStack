/*
 * "main.c", by Sean Soderman
 * Simply tests the 2-3 tree functions I've implemented.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <tree23.h>

#ifndef DEFAULT_INSERTS
#define DEFAULT_INSERTS 100000ULL
#endif

#ifndef DEFAULT_DELETES
#define DEFAULT_DELETES DEFAULT_INSERTS / 2
#endif

/*
 * Runs a standard test of the program using 100,000 
 * randomised insertions and 50,000 deletions.
 */
void treetest(uint64_t num_to_insert, uint64_t num_to_delete, char * filename);

int main(int argc, char * argv[])
{
	if (argc < 3) {
		fprintf(stderr, "No options specified. Will run standard "
					"test.\n");
		fprintf(stderr, "Usage: %s [num_to_insert] [num_to_delete]" 
					" [filename]\n", argv[0]);
		treetest(DEFAULT_INSERTS, DEFAULT_DELETES, NULL);
	} else {
		uint64_t num_to_insert = (uint64_t)atoll(argv[1]);
		uint64_t num_to_delete = (uint64_t)atoll(argv[2]);
		char * filename = argc >= 4 ? argv[3] : NULL;

		treetest(num_to_insert, num_to_delete, filename);
	}
	return 0;
}

void nodecheck(struct tree23_node * n) {
	char * debug_msg = "Value of left ptr: %p\nValue of middle ptr: %p\n"
		"Value of mid_right ptr: %p\nValue of right ptr:%p\n"
		"Value of parent: %p\nldata: %f\nmdata: %f\nrdata:"
		"%f\nis2node: %s\nis3node: %s\nis4node: %s\n";
	char * arr[] = {"false", "true"};

	fprintf(stderr, debug_msg,
		n->left, n->middle, n->mid_right, n->right, n->parent,
		n->ldata, n->mdata, n->rdata, arr[n->is2node],
		arr[n->is3node], arr[n->is4node]);
}

/* Runs a tree test of the program using a user-specified
 * number of insertions/deletions.
 */
void treetest(uint64_t num_to_insert, uint64_t num_to_delete, char * filename)
{
	uint64_t testbuflen = num_to_insert;
	int i = 0;
	time_t seed = time(NULL);
	FILE * fdump = NULL;
	float * test_array;
	clock_t start_time;
	clock_t end_time;

	/* 
	 * Make sure the usr can't try deleting more than was inserted (nothing
	 * would happen anyway, but the test buffer is num_to_insert floats 
	 * long.)
	 */
	num_to_delete = num_to_delete > num_to_insert ? 
                                num_to_insert : num_to_delete;
	if (errno == EOVERFLOW) {
		fprintf(stderr, "It's at least the year 2038! I've probably "
			"got a wife, kids, and a Ph.D! Update your system "
			"please!\n");
		exit(1);
	}
	srand(seed);
	struct tree23_root *t = tree23_root_init();

	/* 
	 * To decrease the overhead of inserting values as much as possible,
	 * fill an array with random numbers first, then insert and delete
	 * them all within two one-line for-loops.
	 */
	test_array = malloc(sizeof(float) * testbuflen);

	/*
	 * If a filename was specified, dump all values that will be deleted
	 * into the newly created file.
	 */
	if (filename != NULL)
		fdump = fopen(filename, "w+");
	for (i; i < testbuflen; i++) {
   		test_array[i] = (float)(rand());
   		if (fdump != NULL)
	      		fprintf(fdump, "%f\n", test_array[i]);
	}

	if (filename != NULL)
   		fclose(fdump);
	start_time = clock();
	for (i = 0; i < testbuflen; i++)
		tree23_insert(test_array[i], t);
	for (i = 0; i < num_to_delete; i++) {
		tree23_erase(test_array[i], t);
	}

	end_time = clock();
	printf("Runtime in clock ticks: %li, seconds: %f\n", 
			(end_time - start_time),
			(float)(end_time - start_time) / CLOCKS_PER_SEC);
	printf("**Tree remnants incoming**\n");

	tree23_print(t->root);
	tree23_deltree(t);
	free(test_array);
}
