#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void* cc_malloc(size_t sz);

struct StringArray 
{
    char** data;
    size_t size;
    size_t max_capacity;
};

struct StringArray new_string_array();

void add_string(struct StringArray* arr, char const* string);



#define DEFINE_NEW_DYN_ARRAY(ARRAY_NAME, TYPE, NEW_FUNC_NAME, ADD_FUNC_NAME) \
struct ARRAY_NAME  \
{ \
    TYPE* data; \
    size_t size; \
    size_t max_capacity; \
}; \
struct ARRAY_NAME NEW_FUNC_NAME(); \
void ADD_FUNC_NAME(struct ARRAY_NAME* arr, TYPE const* value);

#define IMPLEMENT_NEW_DYN_ARRAY(ARRAY_NAME, TYPE, NEW_FUNC_NAME, ADD_FUNC_NAME) \
struct ARRAY_NAME NEW_FUNC_NAME() \
{  \
    return (struct ARRAY_NAME) { \
        .data = cc_malloc(8 * sizeof(TYPE)), \
        .size = 0, \
        .max_capacity = 8 \
    }; \
} \
void ADD_FUNC_NAME(struct ARRAY_NAME* arr, TYPE const* value) \
{ \
    if (arr->size + 1 >= arr->max_capacity) { \
        arr->max_capacity *= 1.4;  \
        arr->data = realloc(arr->data, arr->max_capacity * sizeof(TYPE)); \
    } \
    arr->data[arr->size] = *value; \
    ++arr->size; \
}

struct HashmapElem
{
    const char* key;
    int32_t value;
};

struct HashMap
{
    struct HashmapElem* data;
    size_t capacity;
    size_t size;
};

struct HashMap new_hashmap();
void hashmap_insert(struct HashMap* map, const char* key, int32_t value);
// May return null
int32_t* hashmap_find(struct HashMap* map, const char* key);
// void hashmap_erase(struct HashMap* map, const char* key);

__attribute__((format(printf, 1, 2)))
char* format(char const* format, ...);

#define UNUSED(value) ((void) value)

