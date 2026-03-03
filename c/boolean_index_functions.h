#ifndef BOOLEAN_INDEX_H
#define BOOLEAN_INDEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Максимальные константы
#define MAX_TERM_LENGTH 256
#define MAX_DOC_ID 1000000
#define INITIAL_TERM_CAPACITY 10000
#define INITIAL_POSTING_CAPACITY 100

/*Структура для списка документов. Содержит поля:

    int* doc_ids - id документов.
    int capacity - максимальное количество документов в структуре. Может динамически меняться.
    int size - текущее количество документов в структуре.
*/
typedef struct {
    int* doc_ids;
    int capacity;
    int size;
} PostingList;

/*Структура для терма в индексе. Содержит поля:

    char term[MAX_TERM_LENGTH] - терм.
    PostingList* list - список документов, которые сопоставлены терму.
    int doc_frequency - количество документов, хранящихся в списке PostingList* list. То же самое, что и list->size.
*/
typedef struct {
    char term[MAX_TERM_LENGTH];
    PostingList* list;
    int doc_frequency;
} TermEntry;

/*Основная структура булева индекса. Содержит поля:

    TermEntry* terms - список термов.
    int capacity - максимальное количество термов в структуре. Может динамически меняться.
    int size - текущее количество термов в структуре.
    int total_docs - общее количество документов, которые сопоставлены каждому хранящемуся в структуре терму.
*/
typedef struct {
    TermEntry* terms;
    int capacity;
    int size;
    int total_docs;
} BooleanIndex;

// Функции для работы с булевым индексом

BooleanIndex* create_boolean_index();
void destroy_boolean_index(BooleanIndex* index);
int add_term(BooleanIndex* index, const char* term, int doc_id);
PostingList* get_posting_list(BooleanIndex* index, const char* term);
int load_index_from_file(BooleanIndex* index, const char* filename);
int save_index_to_file(BooleanIndex* index, const char* filename);
void print_index_stats(BooleanIndex* index);

// Функции для работы со списком документов PostingList

PostingList* create_posting_list();
void destroy_posting_list(PostingList* list);
int add_doc_to_posting(PostingList* list, int doc_id);
int posting_contains(PostingList* list, int doc_id);
void print_posting_list(PostingList* list);

// Вспомогательные функции

int hash_string(const char* str);
char* strdup_custom(const char* str);
int compare_terms(const void* a, const void* b);

#endif