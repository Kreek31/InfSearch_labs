#ifndef MONGO_LOADER_H
#define MONGO_LOADER_H

#include "boolean_index_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8192

/* Структура для документа из MongoDB. Содержит поля:

    int id - id документа
    char* title - заголовок документа
    char* text - текст документа
    char* url - ссылка документа
    char* source - название источника
*/
typedef struct {
    int id;
    char* title;
    char* text;
    char* url;
    char* source;
} MongoDocument;

// Функции для работы с MongoDB

int load_documents_from_mongo(MongoDocument** documents, int* count);
int load_documents_from_file(const char* filename, MongoDocument** documents, int* count);
int index_mongo_documents(BooleanIndex* index, const char* data_file, int max_docs);
void print_mongo_document(MongoDocument* doc);
void cleanup_mongo_documents(MongoDocument* documents, int count);

// Вспомогательные функции

char* extract_text_from_html(const char* html);
int tokenize_mongo_document(const char* text, char** tokens, int max_tokens);

#endif