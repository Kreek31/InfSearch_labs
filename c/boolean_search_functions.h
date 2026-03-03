#ifndef BOOLEAN_SEARCH_H
#define BOOLEAN_SEARCH_H

#include "boolean_index_functions.h"

// Перечислитель операторов булева поиска
typedef enum {
    OP_AND,     //логическое и
    OP_OR,      //логическое или
    OP_NOT,     //логическое не
    OP_TERM     //используется для обозначения терма, не является логическим опреатором
} BooleanOperator;

/* Структура для узла булева запроса, хранящего логический оператор или терм. Содержит поля:

    BooleanOperator op - логический оператор или терм из перечислителя BooleanOperator
    char term[MAX_TERM_LENGTH] - терм. Используется только когда op=OP_TERM
    struct BooleanQueryNode* left - левый сосед в дереве булева запроса
    struct BooleanQueryNode* right - правый сосед в дереве булева запроса
    PostingList* result - список документов, являющийся результатом булева поиска для данного узла. Если поиск не выполнялся, содержит NULL
*/
typedef struct BooleanQueryNode {
    BooleanOperator op;
    char term[MAX_TERM_LENGTH];
    struct BooleanQueryNode* left;
    struct BooleanQueryNode* right;
    PostingList* result;
} BooleanQueryNode;

/* Структура для результатов поиска. Содержит поля:

    int* doc_ids - список id документов
    int size - текущее количество хранящихся документов
    int capacity - максимально допустимое количество хранящихся документов
*/
typedef struct {
    int* doc_ids;
    int size;
    int capacity;
} SearchResult;

/* Основная структура булева поиска. Содержит поля:

    BooleanIndex* index - структура индекса BooleanIndex, хранящего термы и соответствующие им документы
    BooleanQueryNode* query_tree -
*/
typedef struct {
    BooleanIndex* index;
    BooleanQueryNode* query_tree;
} BooleanSearch;

// Функции для работы с булевым поиском

BooleanSearch* create_boolean_search(BooleanIndex* index);
void destroy_boolean_search(BooleanSearch* search);

// Построение запроса

BooleanQueryNode* create_term_node(const char* term);
BooleanQueryNode* create_operator_node(BooleanOperator op, BooleanQueryNode* left, BooleanQueryNode* right);
void destroy_query_tree(BooleanQueryNode* node);

// Выполнение поиска

PostingList* execute_query(BooleanSearch* search, BooleanQueryNode* node);
SearchResult* search(BooleanSearch* search, const char* query_str);
void destroy_search_result(SearchResult* result);

// Булевы операции над posting lists

PostingList* intersect_postings(PostingList* list1, PostingList* list2);
PostingList* union_postings(PostingList* list1, PostingList* list2);
PostingList* difference_postings(PostingList* list1, PostingList* list2);

// Вспомогательные функции

void print_search_result(SearchResult* result);
void print_query_tree(BooleanQueryNode* node, int depth);
int is_operator_token(const char* token);
BooleanOperator get_operator(const char* token);
PostingList* create_posting_list_copy(PostingList* original);

#endif