#include "constants.h"

struct value_t {
    char value[VALUE_SIZE];
    int time_create;
    int ttl;
};

typedef struct value_t value_t;

class Key_Value {
public:
    Key_Value();
    ~Key_Value();

    int set(std::string key, std::string value, int ttl);
    int get(std::string key, std::string return_value);
    int delete_key(std::string key);
    void delete_all_ttl();

private:
    int cnt_keys_in_table = 0;
    std::map<int, value_t*> key_value_table;
    std::map<int, int> mutex_id;
    pthread_mutex_t mutexes[MAX_KEY];
    
    std::vector<int> list_empty_mutex;
    pthread_mutex_t mutex_create_delete;
    std::hash<std::string> hash_fn;
};