#include "Key-Value.h"

Key_Value::Key_Value() {
    for (int i = MAX_KEY - 1; i >= 0; i--) {
        list_empty_mutex.push_back(i);
        pthread_mutex_init(&mutexes[i], NULL);
    }
    pthread_mutex_init(&mutex_create_delete, NULL);
}

Key_Value::~Key_Value() {};

int Key_Value::set(std::string key, std::string value) {
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        if (cnt_keys_in_table == MAX_KEY) {
            std::cout << "Нет места" << std::endl;
            return ERROR;
        } else {
            pthread_mutex_lock(&mutex_create_delete);
            mutex_id.insert(std::pair<int, int>(id_key, *(list_empty_mutex.end() - 1)));                
            list_empty_mutex.pop_back();
            key_value_table.insert(std::pair<int, std::string>(id_key, value));
            cnt_keys_in_table++;
            std::cout << "Добавлено: key = " << key << ", " << "value = " << value << std::endl;
            pthread_mutex_unlock(&mutex_create_delete);
            return ADDED;
        }
    } else  {
        pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
        key_value_table[id_key] = value;
        std::cout << "Обновлено: key = " << key << ", " << "value = " << value << std::endl;
        pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
        return UPDATED;
    }
}

int Key_Value::get(std::string key, std::string return_value) {
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        std::cout << "Ошибка получения значения " << key << ": ключ не найден" << std::endl;
        return ERROR;
    } else {
        pthread_mutex_lock(&mutexes[mutex_id[id_key]]);
        return_value = key_value_table[id_key];
        std::cout << "Найдено: key = " << key << ", " << "value = " << return_value << std::endl;
        pthread_mutex_unlock(&mutexes[mutex_id[id_key]]);
        return FOUND;
    }
}

int Key_Value::delete_key(std::string key) {
    pthread_mutex_lock(&mutex_create_delete);
    int id_key = hash_fn(key);
    if (key_value_table.find(id_key) == key_value_table.end()) {
        pthread_mutex_unlock(&mutex_create_delete);
        std::cout << "Ошибка удаления ключа " << key << ": ключ не найден" << std::endl;
        return ERROR;
    } else {
            key_value_table.erase(id_key);
            mutex_id.erase(id_key);
            list_empty_mutex.push_back(id_key);
            cnt_keys_in_table--;
            std::cout << "Удалено: key = " << key << std::endl;
        pthread_mutex_unlock(&mutex_create_delete);
        return DELETED;
    }
}