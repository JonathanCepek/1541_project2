
#include <stdlib.h>
#include <stdio.h>

struct cache_blk_t 
{ /* note that no actual data will be stored in the cache */
  unsigned long tag;
  char valid;
  char dirty;
  unsigned LRU;	/*to be used to build the LRU stack for the blocks in a cache set*/
};

struct cache_t 
{
	// The cache is represented by a 2-D array of blocks. 
	// The first dimension of the 2D array is "nsets" which is the number of sets (entries)
	// The second dimension is "assoc", which is the number of blocks in each set.
  int nsets;					// number of sets
  int blocksize;				// block size
  int assoc;					// associativity
  int mem_latency;				// the miss penalty
  struct cache_blk_t **blocks;	// a pointer to the array of cache blocks
};

static FILE *config_fd;

int log_2(int temp)
{
	int j;
	for (j = 1; j <= 32; j++)
	{
		temp = temp >> 1;
		if (temp == 1)
		{
			return j;
		}
		else if (temp == 0)
		{
			j = 0;
			break;
		}
	}
	return 0;
}

struct cache_t * cache_create(int size, int blocksize, int assoc, int mem_latency)
{
  int i;
  int nblocks = 1;			// number of blocks in the cache
  int nsets = 1;			// number of sets (entries) in the cache

  // YOUR JOB: calculate the number of sets and blocks in the cache
  nblocks = (size*1024)/blocksize;
  nsets = nblocks/assoc;

  struct cache_t *C = (struct cache_t *)calloc(1, sizeof(struct cache_t));
		
  C->nsets = nsets; 
  C->assoc = assoc;
  C->blocksize = blocksize;
  C->mem_latency = mem_latency;

  C->blocks= (struct cache_blk_t **)calloc(nsets, sizeof(struct cache_blk_t *));

  for(i = 0; i < nsets; i++) {
		C->blocks[i] = (struct cache_blk_t *)calloc(assoc, sizeof(struct cache_blk_t));
	}

  return C;
}

int cache_access(struct cache_t *cp, unsigned long address, int access_type)
{
  //
  // Based on "address", determine the set to access in cp and examine the blocks
  // in the set to check hit/miss and update the golbal hit/miss statistics
  // If a miss, determine the victim in the set to replace (LRU). 
  //
  // The function should return the hit_latency, which is 0, in case of a hit.
  // In case of a miss, the function should return mem_latency if no write back is needed.
  // If a write back is needed, the function should return 2*mem_latency.
  // access_type (0 for a read and 1 for a write) should be used to set/update the dirty bit.
  // The LRU field of the blocks in the set accessed should also be updated.
	
  	unsigned int offset_n, index_n, tag_n, offset, index, tag;
  	int i, t1, t2, allocated = 0, maxIndex = -1, currValue;
  	unsigned maxValue = 0;
  	//printf("address: %lu\n", address);
  	offset_n = log_2(cp->blocksize)/log_2(2);
  	index_n = log_2(cp->nsets)/log_2(2);
  	tag_n = 32 - index_n - offset_n;
	
	/*printf("offset_n: %d\n", offset_n);
    printf("index_n: %d\n", index_n);
	printf("tag_n: %d\n", tag_n);*/

  	//break up instruction into necessary bits
  	tag = address >> (32-tag_n);
  	offset = address << (32-offset_n);
  	offset = offset >> (32-offset_n);
  	
  	index = address << (32 - offset_n - index_n);
  	index = index >> (32 - index_n);
  	
	/*printf("\n");
  	printf("address %lu\n", address);
  	printf("tag     %d\n", tag);
  	printf("offset  %d\n", offset);
  	printf("index  %d\n", index);*/


	struct cache_blk_t * check = cp->blocks[index%cp->nsets]; //mod to find the set index
	for (i = 0; i < cp->assoc ; i++)
	{
		if(check[i].valid == 1)
		{
			currValue = check[i].LRU;
			if (check[i].tag == tag)
			{
				//tag found
				if(access_type == 1) printf("HIT\n");
				allocated = 1;
				check[i].dirty = 1;
				currValue = 0;
			}
			if(currValue > maxValue)
			{
				maxValue = currValue;
				maxIndex = i;
			}
			check[i].LRU = currValue + 1;
		}
		else{
			//always allocated
			if(access_type == 1) printf("INVALID\n");
			check[i].tag = tag;
			check[i].valid = 1;
			check[i].dirty = 0;
			check[i].LRU = 1;
			allocated = 1;
			maxIndex = i;
			maxValue = 4294967295;
			return cp->mem_latency;
		}
	}

	if(!allocated)
	{
		check[maxIndex].LRU = 1;
		check[maxIndex].tag = tag;
		check[maxIndex].valid = 1;
		if(check[maxIndex].dirty) {
			if(access_type == 1) printf("dirty\n");
			check[maxIndex].dirty = 0;
			return 2*cp->mem_latency;
		}
		else {
			if(access_type == 1) printf("miss\n");
			return cp->mem_latency;
		}
	}

	return 0
	;
}
