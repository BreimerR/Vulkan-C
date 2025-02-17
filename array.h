//
// Created by brymher on 18/12/24.
//

#ifndef LEARNING_ARRAY_H
#define LEARNING_ARRAY_H

#include "stdlib.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "io.h"
#include "stdbool.h"
#include "vulkan_any.h"
// #include "glib-2.0/glib.h"

typedef struct Uint32SizedMutableArray {
    Any*items;
    size_t size;
    uint32_t count; // Define this else c will assign a memory value that could be very large
} Uint32SizedMutableArray;

Uint32SizedMutableArray *createUint32SizedMutableArray() {
    Uint32SizedMutableArray *array = malloc(sizeof(Uint32SizedMutableArray));
    array->size = 0;
    array->count = 0;
    array->items = malloc(0);

    return array;
}

// This fails on resizing window semaphores and inFlightFences
bool resizeUint32SizedMutableArray(Uint32SizedMutableArray *array, const uint32_t itemCount, const size_t itemSize) {
    if (array == nullptr || array->count == itemCount) return false;

    array->size = itemCount * itemSize;

    if (array->items == nullptr) {
        array->items = malloc(array->size);
        array->count = itemCount;
        return true;
    }

    uint32_t loopCount = array->count;

    if (array->count > itemCount) loopCount = itemCount;

    // Array is downsizing. We need to remove excess items.
    Any*items = malloc(array->size);

    if (items == nullptr) return false;
    uint32_t i = 0;

    for (; i < loopCount; i++) {
        items[i] = array->items + i * itemSize;
    }

    for (; i < array->count ; i++) {
        printLn("Freed item %d", i);
        free(array->items + i * itemSize);
    }

    free(array->items);
    array->items = items;

    array->count = itemCount;

    return true;
}

void addToUint32SizedMutableArray(const size_t item_size, void *item, Uint32SizedMutableArray *array) {
    if (item == NULL) {
        fprintf(stderr, "Can't add null item to array");
        exit(39);
    }

    if (array == NULL) {
        fprintf(stderr, "Empty array passed to Uint32SizedMutableArray");
        exit(39);
    }

    int i = 0;
    array->size = array->size + item_size;

    if (array->items == NULL) {
        array->items = malloc(item_size);
    } else {
        Any*items = malloc(array->size);
        for (; i < array->count; ++i)
            items[i] = array->items[i];

        array->items = items;
    }

    array->items[i] = item;
    array->count = array->count + 1;
}

typedef bool (*Uint32SizedMutableArraySearch)(Uint32SizedMutableArray, uint32_t);

typedef bool (*Uint32SizedMutableArrayValidator)(Any, Uint32SizedMutableArray, uint32_t);

typedef Any (*Uint32SizedMutableArrayRetriever)(uint32_t, Uint32SizedMutableArray);

Any getItemAtIndex(const int index, const Uint32SizedMutableArray *array) {
    if (index < 0 || index > array->count - 1) {
        fprintf(stderr, "Index out of bounce exception, %d", index);
        fflush(stderr);
        exit(39);
    }
    return array->items[index];
}

int getFirstIndexOfItemInUint32SizedMutableArray(
    Any item,
    const Uint32SizedMutableArray array,
    const Uint32SizedMutableArrayValidator comparator
) {
    for (uint32_t i = 0; i < array.count; ++i)
        if (comparator(item, array, i) == true) return (int) i;

    return -1;
}

int findFirstIndexInUint32SizedMutableArray(
    const Uint32SizedMutableArray array,
    const Uint32SizedMutableArraySearch comparator
) {
    for (uint32_t i = 0; i < array.count; ++i)
        if (comparator(array, i) == true) return i;

    return -1;
}

Any getItemAtIndexOrNull(const int index, const Uint32SizedMutableArray *array) {
    if (index < 0 || index > array->count - 1) return NULL;
    return array->items[index];
}

bool uInt32SizedArrayIsEmpty(const Uint32SizedMutableArray array) {
    return array.count == 0;
}

#endif //LEARNING_ARRAY_H
