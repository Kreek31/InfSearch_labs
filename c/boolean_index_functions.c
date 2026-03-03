#include "boolean_index_functions.h"

// Хеш-функция для строк
int hash_string(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % INITIAL_TERM_CAPACITY;
}

//Эта функция выделяет память для строки str и копирует str в выделенную память, возвращая указатель на неё
char* strdup_custom(const char* str) {
    if (!str) return NULL;
    char* dup = malloc(strlen(str) + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

// Сравнение термов для сортировки
int compare_terms(const void* a, const void* b) {
    const TermEntry* term_a = (const TermEntry*)a;
    const TermEntry* term_b = (const TermEntry*)b;
    return strcmp(term_a->term, term_b->term);
}

// Эта функция создает и возвращает указатель на структуру PostingList
PostingList* create_posting_list() {
    PostingList* list = malloc(sizeof(PostingList));
    if (!list) return NULL;
    
    list->capacity = INITIAL_POSTING_CAPACITY;
    list->size = 0;
    list->doc_ids = malloc(list->capacity * sizeof(int));
    
    if (!list->doc_ids) {
        free(list);
        return NULL;
    }
    
    return list;
}

// Эта функция уничтожает объект PostingList* list с корректным освобождением памяти
void destroy_posting_list(PostingList* list) {
    if (list) {
        if (list->doc_ids) {
            free(list->doc_ids);
        }
        free(list);
    }
}

// Эта функция добавляет документ с id равным doc_id в структуру PostingList* list. Возвращает 1 в случае успеха и 0 в случае неудачи.
int add_doc_to_posting(PostingList* list, int doc_id) {
    if (!list) return 0;
    
    // Проверяем, не существует ли уже этот документ
    for (int i = 0; i < list->size; i++) {
        if (list->doc_ids[i] == doc_id) {
            return 1;
        }
    }
    
    // Расширяем массив если необходимо
    if (list->size >= list->capacity) {
        int new_capacity = list->capacity * 2;
        int* new_doc_ids = realloc(list->doc_ids, new_capacity * sizeof(int));
        if (!new_doc_ids) {
            return 0;
        }
        list->doc_ids = new_doc_ids;
        list->capacity = new_capacity;
    }
    
    // Ищем отсортированную позицию вставки документа
    int insert_pos = 0;
    while (insert_pos < list->size && list->doc_ids[insert_pos] < doc_id) {
        insert_pos++;
    }
    
    // Сдвигаем элементы
    for (int i = list->size; i > insert_pos; i--) {
        list->doc_ids[i] = list->doc_ids[i - 1];
    }
    
    list->doc_ids[insert_pos] = doc_id;
    list->size++;
    
    return 1;
}

//Эта функция проверяет наличие документа doc_id в list. Возвращает 1, если документ найден, 0 если найти не удалось и -1 в случае ошибки.
int posting_contains(PostingList* list, int doc_id) {
    if (!list) return -1;
    
    for (int i = 0; i < list->size; i++) {
        if (list->doc_ids[i] == doc_id) {
            return 1;
        }
    }
    return 0;
}

//Эта функция печатает все id, хранящиеся в list
void print_posting_list(PostingList* list) {
    if (!list) {
        printf("NULL posting list\n");
        return;
    }
    
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%d", list->doc_ids[i]);
        if (i < list->size - 1) {
            printf(", ");
        }
    }
    printf("] (size: %d)\n", list->size);
}

//Эта функция создает булев индекс BooleanIndex и возвращает указатель на него
BooleanIndex* create_boolean_index() {
    BooleanIndex* index = malloc(sizeof(BooleanIndex));
    if (!index) return NULL;
    
    index->capacity = INITIAL_TERM_CAPACITY;
    index->size = 0;
    index->total_docs = 0;
    
    index->terms = malloc(index->capacity * sizeof(TermEntry));
    if (!index->terms) {
        free(index);
        return NULL;
    }
    
    // Инициализация термов
    for (int i = 0; i < index->capacity; i++) {
        index->terms[i].term[0] = '\0';
        index->terms[i].list = NULL;
        index->terms[i].doc_frequency = 0;
    }
    
    return index;
}

// Эта функция уничтожает объект BooleanIndex* index с корректным освобождением памяти
void destroy_boolean_index(BooleanIndex* index) {
    if (!index) return;
    
    if (index->terms) {
        for (int i = 0; i < index->size; i++) {
            destroy_posting_list(index->terms[i].list);
        }
        free(index->terms);
    }
    
    free(index);
}

// Эта функция ищет терм среди их множества, хранящегося в структуре BooleanIndex* index. В случае нахождения возвращает индекс терма в структуре BooleanIndex* index, а в случае неудачи возвращает -1.
int find_term_index(BooleanIndex* index, const char* term) {
    if (!index || !term) return -1;
    
    for (int i = 0; i < index->size; i++) {
        if (strcmp(index->terms[i].term, term) == 0) {
            return i;
        }
    }
    
    return -1;
}

// Эта функция добавляет терм в структуру BooleanIndex* index и добавляет этому терму в соответствие документ doc_id. Возвращает 1 в случае успеха и 0 в случае неудачи.
int add_term(BooleanIndex* index, const char* term, int doc_id) {
    if (!index || !term || doc_id < 0) return 0;
    
    // Проверяем, существует ли уже термин
    int term_index = find_term_index(index, term);
    
    if (term_index == -1) {
        // Термин не существует, создаем новый
        if (index->size >= index->capacity) {
            // Расширяем максимальное количество хранящихся термов по необходимости
            int new_capacity = index->capacity * 2;
            TermEntry* new_terms = (TermEntry*)realloc(index->terms, new_capacity * sizeof(TermEntry));
            if (!new_terms) {
                return 0;
            }
            index->terms = new_terms;
            index->capacity = new_capacity;
            
            // Инициализация новых элементов
            for (int i = index->size; i < index->capacity; i++) {
                index->terms[i].term[0] = '\0';
                index->terms[i].list = NULL;
                index->terms[i].doc_frequency = 0;
            }
        }
        
        // Добавляем новый терм
        strncpy(index->terms[index->size].term, term, MAX_TERM_LENGTH - 1);
        index->terms[index->size].term[MAX_TERM_LENGTH - 1] = '\0';
        index->terms[index->size].list = create_posting_list();
        index->terms[index->size].doc_frequency = 0;
        
        if (!index->terms[index->size].list) {
            return 0;
        }
        
        term_index = index->size;
        index->size++;
    }
    
    // Добавляем документ в posting list
    if (add_doc_to_posting(index->terms[term_index].list, doc_id)) {
        index->terms[term_index].doc_frequency++;
        
        // Обновляем общее количество документов если нужно
        if (doc_id >= index->total_docs) {
            index->total_docs = doc_id + 1;
        }
        
        return 1;
    }
    
    return 0;
}

// Эта функция получает спискок документов для терма const char* term, если он имеется в BooleanIndex* index. Возвращает указатель на структуру PostingList, в которой хранятся соответствующие документы
PostingList* get_posting_list(BooleanIndex* index, const char* term) {
    if (!index || !term) return NULL;
    
    int term_index = find_term_index(index, term);
    if (term_index == -1) {
        return NULL;
    }
    
    return index->terms[term_index].list;
}

// Эта функция сохраняет индекс BooleanIndex* index в бинарный файл const char* filename. Возвращает 1 в случае успеха и 0 в случае неудачи.
int save_index_to_file(BooleanIndex* index, const char* filename) {
    if (!index || !filename) return 0;
    
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;
    
    // Сохраняем заголовок
    fwrite(&index->size, sizeof(int), 1, file);
    fwrite(&index->total_docs, sizeof(int), 1, file);
    
    // Сохраняем термины
    for (int i = 0; i < index->size; i++) {
        TermEntry* term = &index->terms[i];
        
        // Сохраняем длину термина и сам термин
        int term_len = strlen(term->term);
        fwrite(&term_len, sizeof(int), 1, file);
        fwrite(term->term, sizeof(char), term_len, file);
        
        // Сохраняем информацию о posting list
        fwrite(&term->doc_frequency, sizeof(int), 1, file);
        fwrite(&term->list->size, sizeof(int), 1, file);
        
        // Сохраняем документы
        if (term->list->size > 0) {
            fwrite(term->list->doc_ids, sizeof(int), term->list->size, file);
        }
    }
    
    fclose(file);
    return 1;
}

// Эта функция загружает индекс из бинарного файла const char* filename. В случае успеха возвращает 1, в случае неудачи 0.
int load_index_from_file(BooleanIndex* index, const char* filename) {
    if (!index || !filename) return 0;
    
    FILE* file = fopen(filename, "rb");
    if (!file) return 0;
    
    // Загружаем заголовок
    fread(&index->size, sizeof(int), 1, file);
    fread(&index->total_docs, sizeof(int), 1, file);
    
    // Проверяем емкость
    if (index->size > index->capacity) {
        index->capacity = index->size;
        index->terms = realloc(index->terms, index->capacity * sizeof(TermEntry));
        if (!index->terms) {
            fclose(file);
            return 0;
        }
    }
    
    // Загружаем термы
    for (int i = 0; i < index->size; i++) {
        TermEntry* term = &index->terms[i];
        
        // Загружаем терм
        int term_len;
        fread(&term_len, sizeof(int), 1, file);
        fread(term->term, sizeof(char), term_len, file);
        term->term[term_len] = '\0';
        
        // Загружаем информацию о posting list
        fread(&term->doc_frequency, sizeof(int), 1, file);
        
        term->list = create_posting_list();
        if (!term->list) {
            fclose(file);
            return 0;
        }
        
        fread(&term->list->size, sizeof(int), 1, file);
        
        // Загружаем документы
        if (term->list->size > 0) {
            if (term->list->size > term->list->capacity) {
                term->list->capacity = term->list->size;
                term->list->doc_ids = (int*)realloc(term->list->doc_ids, 
                                                        term->list->capacity * sizeof(int));
                if (!term->list->doc_ids) {
                    fclose(file);
                    return 0;
                }
            }
            
            fread(term->list->doc_ids, sizeof(int), term->list->size, file);
        }
    }
    
    fclose(file);
    return 1;
}

// Эта функция печатает данные о BooleanIndex* index
void print_index_stats(BooleanIndex* index) {
    if (!index) {
        printf("Index is NULL\n");
        return;
    }
    
    printf("Boolean Index Statistics:\n");
    printf("  Total terms: %d\n", index->size);
    printf("  Total documents: %d\n", index->total_docs);
    printf("  Capacity: %d\n", index->capacity);
    
    int total_postings = 0;
    int max_postings = 0;
    int min_postings = index->size > 0 ? index->terms[0].doc_frequency : 0;
    
    for (int i = 0; i < index->size; i++) {
        int freq = index->terms[i].doc_frequency;
        total_postings += freq;
        if (freq > max_postings) max_postings = freq;
        if (freq < min_postings) min_postings = freq;
    }
    
    printf("  Total postings: %d\n", total_postings);
    printf("  Average postings per term: %.2f\n", 
           index->size > 0 ? (float)total_postings / index->size : 0.0f);
    printf("  Max term postings: %d\n", max_postings);
    printf("  Min term postings: %d\n", min_postings);
}