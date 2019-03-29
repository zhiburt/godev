/*
 * Copyright (c) 2002-2019 "Neo4j,"
 * Neo4j Sweden AB [http://neo4j.com]
 *
 * This file is part of Neo4j.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <limits.h>
#include <memory.h>
#include <string.h>

#include "buffering.h"
#include "mem.h"

BoltBuffer* BoltBuffer_create(int size)
{
    BoltBuffer* buffer = BoltMem_allocate(sizeof(BoltBuffer));
    buffer->size = size;
    buffer->data = BoltMem_allocate((size_t) (buffer->size));
    buffer->extent = 0;
    buffer->cursor = 0;
    return buffer;
}

void BoltBuffer_destroy(BoltBuffer* buffer)
{
    buffer->data = BoltMem_deallocate(buffer->data, (size_t) (buffer->size));
    BoltMem_deallocate(buffer, sizeof(BoltBuffer));
}

void BoltBuffer_compact(BoltBuffer* buffer)
{
    if (buffer->cursor>0) {
        int available = buffer->extent-buffer->cursor;
        if (available<buffer->cursor) {
            memmove(buffer->data, buffer->data+buffer->cursor, (size_t) available);
            buffer->cursor = 0;
            buffer->extent = available;
        }
    }
}

int BoltBuffer_loadable(BoltBuffer* buffer)
{
    int available = buffer->size-buffer->extent;
    return available>INT_MAX ? INT_MAX : available;
}

char* BoltBuffer_load_pointer(BoltBuffer* buffer, int size)
{
    int available = BoltBuffer_loadable(buffer);
    if (size>available) {
        int new_size = buffer->size+(size-available);
        buffer->data = BoltMem_reallocate(buffer->data, (size_t) (buffer->size), (size_t) (new_size));
        buffer->size = new_size;
    }
    int extent = buffer->extent;
    buffer->extent += size;
    return &buffer->data[extent];
}

void BoltBuffer_load(BoltBuffer* buffer, const char* data, int size)
{
    char* target = BoltBuffer_load_pointer(buffer, size);
    memcpy(target, data, size>=0 ? (size_t) (size) : 0);
}

void BoltBuffer_load_i8(BoltBuffer* buffer, int8_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    target[0] = (char) (x);
}

void BoltBuffer_load_u8(BoltBuffer* buffer, uint8_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    target[0] = (char) (x);
}

void BoltBuffer_load_u16be(BoltBuffer* buffer, uint16_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    memcpy_be(&target[0], &x, sizeof(x));
}

void BoltBuffer_load_i16be(BoltBuffer* buffer, int16_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    memcpy_be(&target[0], &x, sizeof(x));
}

void BoltBuffer_load_i32be(BoltBuffer* buffer, int32_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    memcpy_be(&target[0], &x, sizeof(x));
}

void BoltBuffer_load_i64be(BoltBuffer* buffer, int64_t x)
{
    char* target = BoltBuffer_load_pointer(buffer, sizeof(x));
    memcpy_be(&target[0], &x, sizeof(x));
}

void BoltBuffer_load_f64be(BoltBuffer* buffer, double x)
{
    char* target = BoltBuffer_load_pointer(buffer, (int) ((sizeof(x))));
    memcpy_be(&target[0], &x, sizeof(x));
}

int BoltBuffer_unloadable(BoltBuffer* buffer)
{
    return buffer->extent-buffer->cursor;
}

char* BoltBuffer_unload_pointer(BoltBuffer* buffer, int size)
{
    int available = BoltBuffer_unloadable(buffer);
    if (size>available) return NULL;
    int cursor = buffer->cursor;
    buffer->cursor += size;
    if (buffer->cursor==buffer->extent) {
        buffer->extent = 0;
        buffer->cursor = 0;
    }
    return &buffer->data[cursor];
}

int BoltBuffer_unload(BoltBuffer* buffer, char* data, int size)
{
    int available = BoltBuffer_unloadable(buffer);
    if (size>available) return -1;
    memcpy(data, &buffer->data[buffer->cursor], (size_t) (size));
    buffer->cursor += size;
    if (buffer->cursor==buffer->extent) {
        BoltBuffer_compact(buffer);
    }
    return size;
}

int BoltBuffer_peek_u8(BoltBuffer* buffer, uint8_t* x)
{
    if (BoltBuffer_unloadable(buffer)<1) return -1;
    *x = (uint8_t) (buffer->data[buffer->cursor]);
    return 0;
}

int BoltBuffer_unload_u8(BoltBuffer* buffer, uint8_t* x)
{
    if (BoltBuffer_unloadable(buffer)<1) return -1;
    *x = (uint8_t) (buffer->data[buffer->cursor++]);
    return 0;
}

int BoltBuffer_unload_u16be(BoltBuffer* buffer, uint16_t* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}

int BoltBuffer_unload_i8(BoltBuffer* buffer, int8_t* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}

int BoltBuffer_unload_i16be(BoltBuffer* buffer, int16_t* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}

int BoltBuffer_unload_i32be(BoltBuffer* buffer, int32_t* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}

int BoltBuffer_unload_i64be(BoltBuffer* buffer, int64_t* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}

int BoltBuffer_unload_f64be(BoltBuffer* buffer, double* x)
{
    int size_of_x = (int) sizeof(*x);
    if (BoltBuffer_unloadable(buffer)<size_of_x) return -1;
    memcpy_be(x, &buffer->data[buffer->cursor], sizeof(*x));
    buffer->cursor += sizeof(*x);
    return 0;
}
