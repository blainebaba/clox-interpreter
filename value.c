#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "object.h"

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
        case VAL_NIL: printf("nil"); break;
        case VAL_OBJ: printObj(value); break;
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
        case VAL_OBJ:
            Obj* obj1 = AS_OBJ(value1);
            Obj* obj2 = AS_OBJ(value2);
            if (obj1->type != obj2->type) {
                return false;
            }
            switch(obj1->type) {
                case OBJ_STRING:
                    ObjString* s1 = AS_STRING(value1);
                    ObjString* s2 = AS_STRING(value2);
                    if (s1->length != s2->length) {
                        return false;
                    }
                    return memcmp(s1->chars, s2->chars, s1->length) == 0;
                default: return false;
            }
        default: return false;
    }
}