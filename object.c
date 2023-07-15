#include <string.h>

#include "object.h"
#include "memory.h"

#define ALLOCATE_OBJ(type, objType) (type*)allocateObj(sizeof(type), objType)

static Obj* allocateObj(size_t size, ObjType type) {
    Obj* obj = (Obj*)reallocate(NULL, 0, size); // allocate
    obj->type = type;
    return obj;
}

static ObjString* allocateString(const char* chars, int length) {
    ObjString* obj = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    obj->chars = chars;
    obj->length = length;
    return obj;
}

ObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length+1); // allocate
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

ObjString* takeString(const char* chars, int length) {
    // no need to copy again
    return allocateString(chars, length);
}

void printObj(Value value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_STRING: printf(AS_CSTRING(value)); break;
        default: return;
    }
}

