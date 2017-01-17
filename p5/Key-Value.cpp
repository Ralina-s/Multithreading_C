#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <string.h>
#include <vector>
#include <map>
#include <functional>

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

const int MAX_KEY = 1000;
const int MAX_VALUE_LEN = 1024;

struct hashlist_t {
	int hash;
	int n;   // to get data[n]
	struct hashlist_t* next;
	struct hashlist_t* prev;
};

struct data_t {
	char value[MAX_VALUE_LEN];
};

struct info_hashlist_t {
	hashlist_t* begin; 
	hashlist_t* fst_free;    // begin free cell
};

typedef hashlist_t hashlist_t;
typedef data_t data_t;
typedef info_hashlist_t info_hashlist_t;

class SharedMap {
public:
	SharedMap() {

		// hashlist
		key_t key = ftok(".", getpid());
		shmid = shmget(key, MAX_KEY * sizeof(hashlist_t), 0666 | IPC_CREAT);
		void* hashlist_ptr = shmat(shmid, 0, 0);

		hashlist = new(hashlist_ptr) hashlist_t[MAX_KEY];

		for (int i = 1; i < MAX_KEY - 1; i++)
		{
			hashlist[i].next = hashlist + (i + 1);
			hashlist[i].prev = hashlist + (i - 1);
			hashlist[i].n = i;
		}

		hashlist[0].next = hashlist + 1;
		hashlist[0].prev = NULL;
		hashlist[0].n = 0;

		hashlist[MAX_KEY - 1].next = NULL;
		hashlist[MAX_KEY - 1].prev = hashlist + (MAX_KEY - 2);
		hashlist[MAX_KEY - 1].n = MAX_KEY - 1;

		

		// data
		key_t key2 = ftok("./..", getpid());
		shmid2 = shmget(key2, MAX_KEY * sizeof(data_t), 0666 | IPC_CREAT);
		void* data_ptr = shmat(shmid2, 0, 0);

		data = new(data_ptr) data_t[MAX_KEY];

		// info_hashlist
		key_t key3 = ftok("./../..", getpid());
		shmid3 = shmget(key3, sizeof(hashlist_t), 0666 | IPC_CREAT);
		void* info_hashlist_ptr = shmat(shmid3, 0, 0);

		info_hashlist = new(info_hashlist_ptr) info_hashlist_t;

		info_hashlist->begin = NULL;
		info_hashlist->fst_free = hashlist;
	}

	hashlist_t* find(int hash_key) {
		hashlist_t* it = info_hashlist->begin; 
		while ((it != NULL) && (it->hash != hash_key)) {
			it = it->next;
		}
		return it;
	}

	void insert(int hash_key, std::string value) {
		hashlist_t* new_hash = info_hashlist->fst_free;
		new_hash->hash = hash_key;
		if (new_hash->prev == NULL) {
			info_hashlist->begin = new_hash;
		} else {
			new_hash->prev->next = new_hash;
		}
		info_hashlist->fst_free = new_hash->next;
		new_hash->next = NULL;
		memcpy(data[new_hash->n].value, value.c_str(), strlen(value.c_str()) + 1);
	}

	void erase(hashlist_t* id_hash_key) {
		hashlist_t* h = id_hash_key;
		if (info_hashlist->begin == h) {
			info_hashlist->begin = h->next;
			if (h->next != NULL) {
				h->next->prev = h->prev;
				h->prev = info_hashlist->fst_free->prev;
			} else {
				h->prev = NULL;
			}
		} else if (h->next == NULL) {
			h->prev->next = h->next;
			info_hashlist->fst_free->prev = h->prev;
			h->prev = info_hashlist->fst_free->prev;
		} else {
			h->prev->next = h->next;
			h->next->prev = h->prev;
			h->prev = info_hashlist->fst_free->prev;
		}
		h->next = info_hashlist->fst_free;
		info_hashlist->fst_free->prev = h;
		info_hashlist->fst_free = h;
	}

	std::string get_value(int n) {
		return std::string(data[n].value);
	}

	void set_value(int n, std::string value) {
		memcpy(data[n].value, value.c_str(), strlen(value.c_str()) + 1);
	}

	~SharedMap() {
		shmctl(shmid, IPC_RMID, 0);
		shmctl(shmid2, IPC_RMID, 0);
		shmctl(shmid3, IPC_RMID, 0);
	}

private:
	hashlist_t* hashlist;
	info_hashlist_t* info_hashlist;
	data_t* data;
	int shmid;
	int shmid2;
	int shmid3;
};

class Key_Value {
public:
    Key_Value() {
    	// create sem
    	key_t key = ftok("/etc/fstab", getpid());
		sem = semget(key, MAX_KEY, 0666 | IPC_CREAT);

    	union semun {
    		int val;
			struct semid_ds *buf;
			ushort array[MAX_KEY];
		} arg;

        for (int i = MAX_KEY - 1; i >= 0; i--) {
            arg.array[i] = 1;
        }
        semctl(sem, 0, SETALL, arg.array);

        key_t key2 = ftok("/etcr/hosts", getpid());
        sem_creat_delete = semget(key2, 1, 0666 | IPC_CREAT);

        arg.val = 1;
        semctl(sem_creat_delete, 0, SETVAL, arg.val);

        for (int i = 0; i < MAX_KEY; i++) {
        	lockAll[i].sem_num = i;
        	lockAll[i].sem_op = -1;
        	lockAll[i].sem_flg = 0;
        }

        for (int i = 0; i < MAX_KEY; i++) {
        	freeAll[i].sem_num = i;
        	freeAll[i].sem_op = 1;
        	freeAll[i].sem_flg = 0;
        }
    }

    ~Key_Value() {
    	semctl(sem, 0, IPC_RMID, 0);
    	semctl(sem_creat_delete, 0, IPC_RMID, 0);
    }

    int set(std::string key, std::string value) {
        semop(sem_creat_delete, &lock_sem_create, 1);
        int hash_key = hash_fn(key);
        hashlist_t* id_hash_key = key_value_table.find(hash_key);        
        if (id_hash_key == NULL) {
            if (cnt_keys_in_table == MAX_KEY) {
            	semop(sem_creat_delete, &free_sem_create, 1);
                std::cout << "Нет места" << std::endl;
                return -1;
            } else {
            	semop(sem, lockAll, MAX_KEY);
	                key_value_table.insert(hash_key, value);
	                cnt_keys_in_table++;
	                std::cout << "Добавлено: key = " << key << ", " << "value = " << value << std::endl;
            	semop(sem_creat_delete, &free_sem_create, 1);
            	semop(sem, freeAll, MAX_KEY);
            }
        } else {
            semop(sem_creat_delete, &free_sem_create, 1);
			lock_res.sem_num = id_hash_key->n;
            semop(sem, &lock_res, 1);
	            key_value_table.set_value(id_hash_key->n, value);
	            std::cout << "Обновлено: key = " << key << ", " << "value = " << value << std::endl;
	        free_res.sem_num = id_hash_key->n;
            semop(sem, &free_res, 1);
        }
        return 0;
    }

    int get(std::string key, std::string return_value) {
        semop(sem_creat_delete, &lock_sem_create, 1);
    	int hash_key = hash_fn(key);
        hashlist_t* id_hash_key = key_value_table.find(hash_key);
        if (id_hash_key == NULL) {
            semop(sem_creat_delete, &free_sem_create, 1);
            std::cout << "Ошибка получения значения " << key << ": ключ не найден" << std::endl;
            return -1;
        } else {
        	read_res[0].sem_num = id_hash_key->n;
        	read_res[1].sem_num = id_hash_key->n;
            semop(sem, read_res, 2);
            return_value = key_value_table.get_value(id_hash_key->n);
            std::cout << "Найдено: key = " << key << ", " << "value = " << return_value << std::endl;
        }
        semop(sem_creat_delete, &free_sem_create, 1);
        return 0;
    }

    int delete_key(std::string key) {
        semop(sem_creat_delete, &lock_sem_create, 1);
    	int hash_key = hash_fn(key);
        hashlist_t* id_hash_key = key_value_table.find(hash_key);
        if (id_hash_key == NULL) {
            semop(sem_creat_delete, &free_sem_create, 1);
            std::cout << "Ошибка удаления ключа " << key << ": ключ не найден" << std::endl;
            return -1;
        } else {
            semop(sem, lockAll, MAX_KEY);
	            key_value_table.erase(id_hash_key);
	            cnt_keys_in_table--;
	        	std::cout << "Удалено: key = " << key << std::endl;
            semop(sem_creat_delete, &free_sem_create, 1);
            semop(sem, freeAll, MAX_KEY);
        }
        return 0;
    }

private:
    int cnt_keys_in_table = 0;

    SharedMap key_value_table;

    int sem;
    int sem_creat_delete;
	struct sembuf lock_res = {0, -1, 0};
	struct sembuf free_res = {0, 1, 0};
	struct sembuf read_res[2] = {{0, -1, 0}, {0, 1, 0}};
	struct sembuf lock_sem_create = {0, -1, 0};
	struct sembuf free_sem_create = {0, 1, 0};
	struct sembuf read_sem_create[2] = {{0, -1, 0}, {0, 1, 0}};
	struct sembuf lockAll[MAX_KEY];
	struct sembuf freeAll[MAX_KEY];

    std::hash<std::string> hash_fn;
};

int main() {

	Key_Value my_table;
	std::string trash;

    if (!fork()) {
    	if (!fork()) {
			my_table.set("1", "биполярная сойка");
	    	my_table.set("3", "дисперсионный гиппопотам");
	    	my_table.get("4", trash);
			my_table.get("2", trash);
		} else {
			my_table.set("5", "двойственный ленивец");
			my_table.delete_key("3");
			my_table.get("6", trash);
		}
    } else {
    	if (!fork()) {
	    	my_table.set("2", "инверсионная лама");
			my_table.delete_key("2");
			my_table.delete_key("4");
			my_table.set("4", "топологический утконос");	
		} else {
			my_table.set("6", "логарифмический тапир");
			my_table.get("3", trash);
			my_table.delete_key("5");
			my_table.get("1", trash);
		}
    }
	std::cout << std::flush;
	
	return 0;
}
