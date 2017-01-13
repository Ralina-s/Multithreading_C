#include "allocator.h"
#include <iostream>

using namespace std;

std::list<Pointer*> Pointer::all = std::list<Pointer*>();

Pointer::Pointer(void *data, size_t size): data(data), size(size)
{
    Pointer::all.push_back(this);
}

Pointer::Pointer(): data(nullptr), size(0)
{
    //Pointer::all.push_back(this);
}

Pointer::Pointer(Pointer const &p)
{
    data = p.data;
    size = p.size;
    Pointer::all.push_back(this);
}

Pointer& Pointer::operator=(Pointer const &p)
{
    if (!data) {
        Pointer::all.push_back(this);
    }
    data = p.data;
    size = p.size;

    return *this;
}

Pointer::~Pointer()
{
    auto it = Pointer::all.begin();
    while (it != Pointer::all.end() && (*it) != this) {
        it++;
    }
    if (it != Pointer::all.end()) {
        //std::cout << "erase" << std::endl;
        Pointer::all.erase(it);
    }
}

Allocator::Allocator(void *base, size_t size)
{
    this->base = base;
    this->size = size;
    freeblocks.push_back(freeblock_t(base, size));
}

Pointer Allocator::alloc(size_t N)
{
    auto it = freeblocks.begin();
    while (it != freeblocks.end() && it->size < N) {
        it++;
    }
    if (it == freeblocks.end()) {
        throw(AllocError(AllocErrorType::NoMemory, "no empty memory for corently data"));
    }
    void *addr = it->data;
    if (it->size == N) {
        freeblocks.erase(it);
    } else {
        it->data = static_cast<void*>(static_cast<char*>(it->data) + N);
        it->size -= N;
    }
    Pointer res(addr, N);
    //pointers.push_back(&res);
    //all.push_back(&res);
    //std::cout << (**(pointers.begin())).data << std::endl;

    return res;
}

void copy(Pointer p, Pointer q)
{
    char *p_data = static_cast<char*>(p.data);
    char *q_data = static_cast<char*>(q.data);
    for (int i = 0; i < p.size; i++) {
        p_data[i] = q_data[i];
    }
}

void Allocator::realloc(Pointer &p, size_t N)
{
    if (!p.data) {
        p = alloc(N);
        return;
    }
    if (N < p.size) {
        Pointer tmp = Pointer(static_cast<char*>(p.data) + N, p.size - N);
        free(tmp);
        p.size = N;
        return;
    }
    Pointer q = p;
    free(p);
    p = alloc(N);
    copy(p, q);
}

void Allocator::free(Pointer &p)
{
    if (p.data < base || static_cast<char*>(p.data) + p.size > static_cast<char*>(base) + size) {
        throw(AllocError(AllocErrorType::InvalidFree, "free error"));
    }
    auto it = freeblocks.begin();
    for (; it != freeblocks.end(); it++) {
        if (static_cast<char*>(it->data) + it->size > static_cast<char*>(p.data) &&
            static_cast<char*>(it->data) <= static_cast<char*>(p.data)) {
                throw(AllocError(AllocErrorType::InvalidFree, "free error"));

        }
        if (static_cast<char*>(it->data) + it->size == static_cast<char*>(p.data)) {
            it->size += p.size;
            break;
        }
    }

    auto it2 = Pointer::all.begin();
    for (; it2 != Pointer::all.end(); it2++) {
        if ((*it2) == &p) {
            Pointer::all.erase(it2);
            break;
        }
    }

    auto first = it;
    if (it == freeblocks.end()) {
        it = freeblocks.begin();
        while (it != freeblocks.end() && it->data <= p.data) {
            it++;
        }
        first = freeblocks.insert(it, freeblock_t(p.data, p.size));
    }
    it = freeblocks.begin();
    for (; it != freeblocks.end(); it++) {
        if (static_cast<char*>(p.data) + p.size == static_cast<char*>(it->data)) {
            first->size += it->size;
            freeblocks.erase(it);
            break;
        }
    }
    p.size = 0;
    p.data = nullptr;
}

void Allocator::defrag()
{
    auto it = freeblocks.begin();
    char *next_memory = static_cast<char*>(it->data) + it->size;
    if (next_memory != static_cast<char*>(base) + size) {
        size_t size_next;
        auto inextfree = freeblocks.begin();
        if (++inextfree == freeblocks.end()) {
            size_next = (static_cast<char*>(base) - next_memory) / sizeof(char);
        } else {
            size_next = (static_cast<char*>(inextfree->data) - next_memory) / sizeof(char);
        }

        char *last = next_memory;
        Pointer tmp(next_memory, size_next);
        realloc(tmp, tmp.size);
        auto it2 = Pointer::all.begin();
        for (; it2 != Pointer::all.end(); it2++) {
            if (static_cast<char*>((*it2)->data) >= last &&
                static_cast<char*>((*it2)->data) < static_cast<char*>(last + size_next)) {
                (*it2)->data = static_cast<char*>((*it2)->data) - (last - static_cast<char*>(tmp.data));
            }
        }

        defrag();
    }
}

std::string Allocator::dump() { return ""; }
