#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void* cc_malloc(size_t sz)
{
    void* alloc = calloc(1, sz); // Dealing only with 0s is easier and can be abused :)
    assert(alloc);
    return alloc;
}

struct StringArray new_string_array()
{
    return (struct StringArray) {
        .data = cc_malloc(8 * sizeof(char*)),
        .size = 0,
        .max_capacity = 8
    };
}


void add_string(struct StringArray* arr, char const* string)
{
    if (arr->size + 1 >= arr->max_capacity) {
        arr->max_capacity *= 1.4; 
        arr->data = realloc(arr->data, arr->max_capacity * sizeof(char*));
        assert(arr->data);
    }
    arr->data[arr->size] = strdup(string);
    ++arr->size;
}

// Hashing function implemantions from: https://stackoverflow.com/questions/7666509/hash-function-for-string
static uint32_t murmurOAAT32(const char* key)
{
  uint32_t h = 3323198485ul;
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }
  return h;
}

// static uint64_t murmurOAAT64(const char* key)
// {
//   uint64_t h = 525201411107845655ull;
//   for (;*key;++key) {
//     h ^= *key;
//     h *= 0x5bd1e9955bd1e995;
//     h ^= h >> 47;
//   }
//   return h;
// }

static const int INITAL_HASHMAP_SIZE = 32;

struct HashMap new_hashmap()
{

    return (struct HashMap) {
        .capacity = INITAL_HASHMAP_SIZE,
        .size = 0,
        .data = cc_malloc(INITAL_HASHMAP_SIZE * sizeof(struct HashmapElem))
    };
}

static void reallocate_hashmap(struct HashMap* map, size_t new_capacity)
{
    struct HashmapElem* new_data = cc_malloc(new_capacity * sizeof(struct HashmapElem));
    for (size_t idx = 0; idx < map->capacity; ++idx)
    {
        struct HashmapElem* item = &map->data[idx];
        if (item->key == NULL) continue;
        size_t new_index = murmurOAAT32(item->key) % new_capacity;
        while (new_data[new_index].key != NULL) new_index = (new_index + 1) % new_capacity;
        new_data[new_index] = *item;
    }
    free(map->data);
    map->data = new_data;
    map->capacity = new_capacity;
}

void hashmap_insert(struct HashMap* map, const char* key, int32_t value)
{
    float load_factor = (float) map->size / map->capacity;
    if (load_factor > 0.7)
    {
        reallocate_hashmap(map, map->size * 1.4);
    }
    size_t index = murmurOAAT32(key) % map->capacity;
    while (map->data[index].key != NULL) index = (index + 1) % map->capacity;
    map->data[index] = (struct HashmapElem){ .key = strdup(key), .value = value};
}

int32_t* hashmap_find(struct HashMap* map, const char* key)
{
    size_t index = murmurOAAT32(key) % map->capacity;
    while (map->data[index].key != NULL)
    {
        struct HashmapElem* elem = &map->data[index];
        if (strcmp(elem->key, key) == 0)
        {
            return &elem->value;
        }
        index = (index + 1) % map->capacity;
    }
    return NULL;
}

__attribute__((format(printf, 1, 2)))
char* format(char const* format, ...)
{
    va_list args;
    va_start(args, format);
    size_t const size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    assert(size > 1);
    char* message = cc_malloc(size);
    va_start(args, format);
    vsnprintf(message, size, format, args);
    va_end(args);
    return message;
}

