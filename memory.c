#include <stdlib.h>

#include "memory.h"
#include "vm.h"
#include "object.h"

// this function can allocate and de-allocate memory.
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

static void freeObject(Obj* obj) {
    switch(obj->type) {
        case OBJ_STRING:
            ObjString* objString = (ObjString*)obj;
            FREE_ARRAY(char, objString->chars, objString->length + 1);
            FREE(ObjString, objString);
            break;
        default: return;
    }
}

void freeObjects() {
    Obj* obj = vm.objects;
    while (obj != NULL) {
        Obj* next = obj->next;
        freeObject(obj);
        obj = next;
    }
}