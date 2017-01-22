#include "constants.h"

class Key_Value {
public:
    Key_Value();
    ~Key_Value();

    int set(std::string key, std::string value);
    int get(std::string key, std::string return_value);
    int delete_key(std::string key);

private:
    int cnt_keys_in_table = 0;
    std::map<int, std::string> key_value_table;
    std::map<int, int> mutex_id;
    pthread_mutex_t mutexes[MAX_KEY];
    
    std::vector<int> list_empty_mutex;
    pthread_mutex_t mutex_create_delete;
    std::hash<std::string> hash_fn;
};