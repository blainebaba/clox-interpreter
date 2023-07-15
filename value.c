#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity == array->count) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
    switch(value.type) {
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
        case VAL_BOOL: printf(AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL: printf("nil");
        default: return;
    }
}

// can't use memcmp(), because value of unused bits are undefined.
bool valueEqual(Value value1, Value value2) {
    if (value1.type != value2.type) {
        return false;
    }
    switch(value1.type) {
        case VAL_BOOL: return AS_BOOL(value1) == AS_BOOL(value2);
        case VAL_NIL: return true;
        case VAL_NUMBER: return AS_NUMBER(value1) == AS_NUMBER(value2);
        default: return false;
    }
}