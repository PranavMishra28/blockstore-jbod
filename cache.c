#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

// Extra global variables created
int cache_created = 0;

int cache_create(int num_entries)
{
  if (num_entries >= 2 && num_entries <= 4096 && cache_created == 0)
  {
    cache = (cache_entry_t *)calloc(num_entries, sizeof(cache_entry_t)); // Assigning memory through calloc
    cache_size = num_entries;
    cache_created = 1;
    return 1;
  }
  return -1;
}

int cache_destroy(void)
{
  if (cache_created == 1)
  {
    free(cache);
    cache = NULL;
    cache_size = 0;
    cache_created = 0;
    
    return 1;

  }
  return -1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf){

  if(cache != NULL){
    if(buf == NULL){
      return -1;
    }
    if(block_num < 0 || block_num >= 256){
      return -1;
    }
    if(disk_num < 0 || disk_num >= 16){
      return -1;
    }
  }

  for (int i = 0; i < cache_size; i++)
  {
    num_queries++; // Whether its a hit or miss we increment num_queries

    if (cache[i].valid == true && cache[i].disk_num ==disk_num && cache[i].block_num == block_num)
    {
      memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
      num_hits++;
      
      clock += 1;
      cache[i].access_time = clock;
      return 1;
    }
  }
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf){
  // num_queries++;

  // If entry exists in cache, updates its block content with buf
  if(cache != NULL){
    for(int i = 0; i < cache_size; i++){
      if (cache[i].valid == true && cache[i].disk_num == disk_num && cache[i].block_num == block_num){
        memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
        num_hits++;
        
        clock += 1;
        cache[i].access_time = clock;
      }
    }
  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf){
  // num_queries++;

  // Primary checks
  if(cache != NULL){
    if(buf == NULL){
      return -1;
    }
    if(block_num < 0 || block_num >= 256){
      return -1;
    }
    if(disk_num < 0 || disk_num >= 16){
      return -1;
    }

  // Checking if the cache is valid or not and incrementing access time
  for(int i = 0; i < cache_size; i++){

    if(cache[i].valid == false){
      cache[i].disk_num = disk_num;
      cache[i].block_num = block_num;

      clock += 1;
      cache[i].access_time = clock;

      memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
      cache[i].valid = true;

      return 1;
    }

    // Performing lookup
    if (cache[i].valid == true && cache[i].disk_num == disk_num && cache[i].block_num == block_num)
    {
      return -1;
    }
  }

  // Implementing cache LRU policy by targeting lowest access time
  int lowestAccessTime = cache[0].access_time;
  for(int i = 0; i < cache_size; i++){
    if (cache[i].access_time < lowestAccessTime)
    {
      lowestAccessTime = cache[i].access_time;
    }
  }

  // Linear Search - Helps in implementation by removing elemnt with lowest access time
  for (int i = 0; i < cache_size; ++i)
  {
    if (cache[i].access_time == lowestAccessTime)
    {
      cache[i].disk_num = disk_num;
      cache[i].block_num = block_num;

      clock += 1;
      cache[i].access_time = clock;

      memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
      return 1;
    }
  }
  }
  return -1; 
}

bool cache_enabled(void)
{
  if (cache_created == 1 && cache != NULL)
  {
    return true;
  }
  return false;
}

void cache_print_hit_rate(void)
{
  fprintf(stderr, "Hit rate: %5.3f%%\n", 100 * (float)num_hits / num_queries);
}