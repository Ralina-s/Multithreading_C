#include <stdexcept>
#include <string>
#include <list>

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {

public:
    void *data;
    size_t size;
    Pointer(void *data, size_t size);
    Pointer();
    void *get() const { return data; }
    ~Pointer();
    Pointer(Pointer const &p);
    Pointer& operator=(Pointer const &p);

    static std::list<Pointer*> all;
};

class freeblock_t
{
public:
    void *data;
    size_t size;
    freeblock_t(void *data, size_t size): data(data), size(size) {}
    freeblock_t(): data(nullptr), size(0) {}
};

class Allocator {
private:

    size_t size;
    std::list<freeblock_t> freeblocks;
    std::list<Pointer*> pointers;
public:
    void *base;
    Allocator(void *base, size_t size);

    Pointer alloc(size_t N);// { return Pointer(); }
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    std::string dump();
};
