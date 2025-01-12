#pragma once

#include "../core/text.h"


u32 getSizeInBytes(const Font &font) {
    return sizeof(Font);
}

bool allocateMemory(Font &font, memory::MonotonicAllocator *memory_allocator) {
    u32 size = getSizeInBytes(font);
    if (size > (memory_allocator->capacity - memory_allocator->occupied)) return false;
//    image.content = (T*)memory_allocator->allocate(size);
    return true;
}

void writeHeader(const Font &font, void *file) {}
void readHeader(const Font &font, void *file) {}

void readContent(Font &font, void *file) {
    os::readFromFile((void*)&font, getSizeInBytes(font), file);
}

void writeContent(const Font &font, void *file) {
    os::writeToFile((void*)&font, getSizeInBytes(font), file);
}