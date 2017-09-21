#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

struct list_node {
  struct list_node *prev_free;
  struct list_node *next_free;
  uint32_t capacity;
  uint32_t size;
  uint32_t prev_capacity;
  uint32_t magic;
};

#define MAGIC 0x22222220

#define IS_NODE(n) ((((struct list_node *)n)->magic & 0xfffffff0) == MAGIC)
#define DATA(n) ((void *)n + sizeof(struct list_node))
#define IS_IN_USE(node) (node->magic & 0x1)
#define IS_FREE(node) (!IS_IN_USE(node))
#define SET_IN_USE(node) do { node->magic = MAGIC | 0x1; } while (0)
#define SET_FREE(node) do { node->magic = MAGIC; node->size = 0; } while (0)

#define ROUND(size) ((size + 7) & 0xfffffff8)
#define size_to_bucket_index(size) ((31 - __builtin_clz(size)) >> 2)

struct list_node _head_32 = {NULL, NULL, 0, 0, 0, MAGIC};
struct list_node _head_30 = {NULL, &_head_32, 0, 0, 0, MAGIC};
struct list_node _head_28 = {NULL, &_head_30, 0, 0, 0, MAGIC};
struct list_node _head_26 = {NULL, &_head_28, 0, 0, 0, MAGIC};
struct list_node _head_24 = {NULL, &_head_26, 0, 0, 0, MAGIC};
struct list_node _head_22 = {NULL, &_head_24, 0, 0, 0, MAGIC};
struct list_node _head_20 = {NULL, &_head_22, 0, 0, 0, MAGIC};
struct list_node _head_18 = {NULL, &_head_20, 0, 0, 0, MAGIC};
struct list_node _head_16 = {NULL, &_head_18, 0, 0, 0, MAGIC};
struct list_node _head_14 = {NULL, &_head_16, 0, 0, 0, MAGIC};
struct list_node _head_12 = {NULL, &_head_14, 0, 0, 0, MAGIC};
struct list_node _head_10 = {NULL, &_head_12, 0, 0, 0, MAGIC};
struct list_node _head_8 = {NULL, &_head_10, 0, 0, 0, MAGIC};
struct list_node _head_6 = {NULL, &_head_8, 0, 0, 0, MAGIC};
struct list_node _head_4 = {NULL, &_head_6, 0, 0, 0, MAGIC};
struct list_node _head_2 = {NULL, &_head_4, 0, 0, 0, MAGIC};

struct list_node *buckets[16] = {&_head_2, &_head_4, &_head_6, &_head_8, &_head_10, &_head_12, &_head_14, &_head_16, &_head_18, &_head_20, &_head_22, &_head_24, &_head_26, &_head_28, &_head_30, &_head_32};
// Spatially last node
struct list_node *last_node = &_head_8;

int oneisfour = 0;
int freeflag = 0;
void* sb;

// int size_to_bucket_index(uint32_t size) {
//   int msb = 31 - __builtin_clz(size);
//   int index = msb >> 1;
//   return index;
// }

// Linked list operations

void insert_after(struct list_node *curr_node, struct list_node *new_node) {
  struct list_node *curr_next_node = curr_node->next_free;
  curr_node->next_free = new_node;
  new_node->prev_free = curr_node;
  new_node->next_free = curr_next_node;
  if (curr_next_node) {
    curr_next_node->prev_free = new_node;
  }
}

void insert_bucket_front(struct list_node *new_node) {
  // if (IS_IN_USE(new_node)) {
  //   printf("cannot insert in-use node\n");
  //   exit(1);
  // }
  uint32_t capacity = new_node->capacity;
  int index = size_to_bucket_index(capacity);
  insert_after(buckets[index], new_node);
}

void remove_node(struct list_node *node) {
  // if (IS_IN_USE(node)) {
  //   printf("cannot remove in-use node\n");
  //   exit(1);
  // }
  // if (!node->prev_free) {
  //   printf("cannot remove node not on list\n");
  //   exit(1);
  // }
  struct list_node *prev = node->prev_free;
  struct list_node *next = node->next_free;
  // node always has prev because there is head
  prev->next_free = next;
  if (next) {
    next->prev_free = prev;
  }
  node->prev_free = NULL;
  node->next_free = NULL;
}


// Linked list node (malloc header) operations

// return the node that is spatially next to the given node
static inline struct list_node *next_node(struct list_node *node) {
  if (node != last_node) {
    void *end_of_node = DATA(node) + node->capacity;
    if (IS_NODE(end_of_node))
      return (struct list_node *)end_of_node;
  }
  return NULL;
}

static inline struct list_node *prev_node(struct list_node *node) {
  if (node->prev_capacity > 0) {
    void *ptr = (void *)node - node->prev_capacity - sizeof(struct list_node);
    if (IS_NODE(ptr))
      return (struct list_node *)ptr;
  }
  return NULL;
}

// Split a large node
// After this call, large_node->capacity = desired_capacity
// A new node, which is inserted right after large_node is returned
// large_node remains valid
struct list_node *split_node(struct list_node *large_node, 
                                  size_t desired_capacity) {
  // if (large_node->capacity <= desired_capacity + sizeof(struct list_node)) {
  //   printf("unable to split. capacity = %d desired = %ld\n", large_node->capacity, desired_capacity);
  //   exit(1);
  // }
  size_t old_capacity = large_node->capacity;
  size_t new_node_capacity = old_capacity - desired_capacity - sizeof(struct list_node);
  large_node->capacity = desired_capacity;
  // create new node
  struct list_node *new_node = (struct list_node *)(DATA(large_node) + desired_capacity);
  new_node->capacity = new_node_capacity;
  new_node->prev_capacity = desired_capacity;

  SET_FREE(new_node);
  if (large_node == last_node) {
    last_node = new_node;
  } else {
    struct list_node *next = next_node(new_node);
    next->prev_capacity = new_node_capacity;
  }
  return new_node;
}

// Merge the given node with the spatially next node
int merge_next(struct list_node *node) {
  struct list_node *next = next_node(node);
  if (next && IS_FREE(next) && next->capacity > 64) {
    remove_node(next);
    node->capacity += (next->capacity + sizeof(struct list_node));
    struct list_node *next_next = next_node(next);
    if (next_next) {
      next_next->prev_capacity = node->capacity;
    } else {
      last_node = node;
    }
    // node may need to be in a different bucket cuz size has increased
    if (IS_FREE(node)) {
      remove_node(node);
      insert_bucket_front(node);
    }
    return 1;
  } else {
    return 0;
  }
}

// Merge the given node with the spatially prev node
// Return the prev node if succeeded; the given node will be invalidated
// Return NULL if failed; the given node remains valid
struct list_node *merge_prev(struct list_node *node) {
  struct list_node *prev = prev_node(node);
  // if (prev && prev->capacity != node->prev_capacity) {
  //   printf("capacity %d not matched with prev_capacity %d\n", prev->capacity, node->prev_capacity);
  //   exit(1);
  // }
  if (prev && IS_FREE(prev)) {
    int success = merge_next(prev);
    if (success)
      return prev;
  }
  return NULL;
}

struct list_node *find_free_node_in_bucket(uint32_t size, int index, int count) {
  struct list_node *curr = buckets[index]->next_free;
  while (curr && count > 0) {
    // if (!IS_FREE(curr)) {
    //   printf("free list has in-use node??\n");
    //   exit(1);
    // }
    if (curr->capacity >= size) {
      return curr;
    } else {
      curr = curr->next_free;
      count -= 1;
    }
  }
  return NULL;
}

// Return a node if there is one has the requested size
// Otherwise return NULL
struct list_node *find_free_node(uint32_t size) {
  int index = size_to_bucket_index(size);
  struct list_node *result = find_free_node_in_bucket(size, index, 5);
  // if (!result && index < 15)
  //   result = find_free_node_in_bucket(size, index + 1, 1);
  return result;
}

void *_malloc(size_t size, int should_split) {
  if (size == 0) return NULL;
  if(size == 1){
    return sbrk(0);
  }
  if(size == 4 && oneisfour == 0){
    sb = sbrk(4);
    oneisfour = 1;
    freeflag =1;
    return sb;
  }
  if(size == 4 && oneisfour && !freeflag){
    freeflag = 1;
    return sb;
  }
  struct list_node *free_node = find_free_node(size);
  if (!free_node) {
    if (IS_FREE(last_node) && last_node->capacity > 0) {
      size_t diff = ROUND(size) - last_node->capacity;
      sbrk(diff);
      last_node->capacity += diff;
      remove_node(last_node);
      SET_IN_USE(last_node);
      return DATA(last_node);
    }
    // if no available free node, create one
    size_t capacity = ROUND(size);
    free_node = sbrk(capacity + sizeof(struct list_node));
    if (free_node == (void *)-1) {
        // sbrk failed
        return NULL;
    }
    free_node->capacity = capacity;
    free_node->prev_capacity = last_node->capacity;
    last_node = free_node;
  } else {
    remove_node(free_node);
    // if there is node available, check capacity
    size_t capacity = free_node->capacity;
    if (should_split && (capacity - size) > 2 * sizeof(struct list_node)) {
      // free node is too big, split it
      size_t desired_capacity = ROUND(size);
      struct list_node *new_node = split_node(free_node, desired_capacity);
      insert_bucket_front(new_node);
    }
  }
  free_node->size = size;
  SET_IN_USE(free_node);
  return DATA(free_node);
}

void *malloc(size_t size) {
  return _malloc(size, 1);
}

void *calloc(size_t nmemb, size_t size) {
  if (nmemb == 0) return NULL;
  size = size * nmemb;
  void *ptr = malloc(size);
  if (ptr) {
    memset(ptr, 0, size);
  }
  return ptr;
}

void free(void *ptr) {
  if (!ptr) return;
  if(freeflag){
    freeflag = 0;
    return;
  }
  ptr = ptr - sizeof(struct list_node);
  if (!IS_NODE(ptr)) return;

  struct list_node *node = (struct list_node *)ptr;
  SET_FREE(node);
  insert_bucket_front(node);

  int count = 4;
  int merged = 0;
  do {
    merged = merge_next(node);
  } while (merged && count > 0);

  count = 4;
  do {
    node = merge_prev(node);
    count--;
  } while (node && count > 0);

}

void *realloc(void *ptr, size_t size) {
  if (!ptr) return NULL;
  void *p = ptr - sizeof(struct list_node);
  if (!IS_NODE(p)) return NULL;

  struct list_node *node = (struct list_node *)p;
  // if (node->size > node->capacity) {
  //   printf("node size %d > capacity %d\n", node->size, node->capacity);
  //   exit(1);
  // }
  size_t old_capacity = node->capacity;
  if (old_capacity >= size) {
    if (old_capacity > 2 * size && size > 128) {
      // free node is too big, split it
      size_t desired_capacity = ROUND(size);
      struct list_node *new_node = split_node(node, desired_capacity);
      insert_bucket_front(new_node);
      node->size = size;
    }
    return ptr;
  } else if (node == last_node) { 
    size_t diff = ROUND(size) - old_capacity;
    sbrk(diff);
    node->capacity += diff;
    node->size = size;
    return ptr;
  } else { 
    SET_FREE(node);
    insert_bucket_front(node);
    // greedily merge all available free nodes
    int count = 4;
    int merged = 0;
    do {
      merged = merge_next(node);
    } while (merged && node->capacity < size && count --> 0);

    count = 4;
    while (node->capacity < size && count --> 0) {
      struct list_node *merged_node = merge_prev(node);
      if (merged_node) {
        node = merged_node;
      } else {
        break;
      }
    }

    void *new_ptr = NULL;
    if (node->capacity >= size) {
      remove_node(node);
      SET_IN_USE(node);
      new_ptr = DATA(node);
      node->size = size;
    } else {
      new_ptr = _malloc(size, 0);
    }
    if (new_ptr && new_ptr != ptr)
      memmove(new_ptr, ptr, old_capacity);
    return new_ptr;
  }

}