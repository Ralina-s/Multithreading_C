#include "Key-Value.h"

Key_Value::Key_Value() {
    for (int i = MAX_KEY - 1; i >= 0; i--) {
        list_empty_mutex.push_back(i);
        pthread_mutex_init(&mutexes[i], NULL);
    }
    pthread_mutex_init(&mutex_create_delete, NULL);
}

Key_Value::~Key_Value() {};

int Key_Value::set(std::string key, std::string value, int ttl) {
    pthread_mutex_lock(&mutex_create_delete);
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        if (cnt_keys_in_table == MAX_KEY) {
            pthread_mutex_unlock(&mutex_create_delete);
            // std::cout << "Нет места" << std::endl;
            return ERROR;
        } else {
            // pthread_mutex_lock(&mutex_create_delete);
                value_t* new_value = (value_t*) calloc (1, sizeof(value_t));
                memcpy(new_value->value, value.c_str(), VALUE_SIZE);
                new_value->time_create = clock();
                new_value->ttl = ttl;

                mutex_id.insert(std::pair<int, int>(id_key, *(list_empty_mutex.end() - 1)));                
                list_empty_mutex.pop_back();
                key_value_table.insert(std::pair<int, value_t*>(id_key, new_value));
                cnt_keys_in_table++;
                // std::cout << "Добавлено: key = " << key << ", " << "value = " << value << std::endl;
            pthread_mutex_unlock(&mutex_create_delete);
            return ADDED;
        }
    } else  {
        pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
            value_t* found_value = key_value_table[id_key];
            memcpy(found_value->value, value.c_str(), VALUE_SIZE);
            found_value->time_create = clock();
            found_value->ttl = ttl;
            // std::cout << "Обновлено: key = " << key << ", " << "value = " << value << std::endl;
        pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
        pthread_mutex_unlock(&mutex_create_delete);
        return UPDATED;
    }
}

int Key_Value::get(std::string key, std::string& return_value) {
    pthread_mutex_lock(&mutex_create_delete);
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        pthread_mutex_unlock(&mutex_create_delete);
        // std::cout << "Ошибка получения значения " << key << ": ключ не найден" << std::endl;
        return ERROR;
    } else {
        pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
            return_value = key_value_table[id_key]->value;
        pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
        pthread_mutex_unlock(&mutex_create_delete);
        return FOUND;
    }
}

int Key_Value::delete_key(std::string key) {
    pthread_mutex_lock(&mutex_create_delete);
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        pthread_mutex_unlock(&mutex_create_delete);
        // std::cout << "Ошибка удаления ключа " << key << ": ключ не найден" << std::endl;
        return ERROR;
    } else {
            pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
                free(key_value_table[id_key]);
                key_value_table.erase(id_key);
                list_empty_mutex.push_back(id_key);
                cnt_keys_in_table--;
            pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
                mutex_id.erase(id_key);
            // std::cout << "Удалено: key = " << key << std::endl;
        pthread_mutex_unlock(&mutex_create_delete);
        return DELETED;
    }
}

void Key_Value::delete_all_ttl() {
    pthread_mutex_lock(&mutex_create_delete);
        for (auto it = key_value_table.cbegin(); it != key_value_table.cend(); ) {
            value_t* cur_value = it->second;
            if ((clock() - cur_value->time_create)  / (double)CLOCKS_PER_SEC > cur_value->ttl) {
                int id_key = it->first;
                pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
                    std::cout << "Удалено: value = " << std::string(cur_value->value) << " – время вышло" << std::endl;
                    free(key_value_table[id_key]);
                    key_value_table.erase(it++);
                    list_empty_mutex.push_back(id_key);
                    cnt_keys_in_table--;
                pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
                    mutex_id.erase(id_key);
            } else {
                ++it;
            }
        }
    pthread_mutex_unlock(&mutex_create_delete);
}
