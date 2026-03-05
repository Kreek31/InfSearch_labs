#include "document_functions.h"
#include <ctype.h>

/*Эта функция загружает документы из файла "test_mongo_data.txt" в структуру MongoDocument** documents, записывая в int* count количесвто загрузок. В случае успеха возвращает 1, а в случае неудачи - 0.
Формат документов в файле: "id|title|text|url|source\n"
*/
int load_documents_from_mongo(MongoDocument** documents, int* count) {
    return load_documents_from_file("mongo_export.txt", documents, count);
}

/* Эта функция загружает документы из файла const char* filename в структуру MongoDocument** documents, записывая в int* count количесвто загрузок. В случае успеха возвращает 1, а в случае неудачи - 0.
Формат документов в файле: "id|title|text|url|source\n"
*/
int load_documents_from_file(const char* filename, MongoDocument** documents, int* count) {
    if (!filename || !documents || !count) return 0;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file: %s\n", filename);
        return 0;
    }
    
    int lines = 0;
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), file)) {
        lines++;
    }
    rewind(file);
    
    *documents = (MongoDocument*)malloc(lines * sizeof(MongoDocument));
    if (!*documents) {
        fclose(file);
        return 0;
    }
    
    int doc_count = 0;
    while (fgets(buffer, sizeof(buffer), file) && doc_count < lines) {
        char* token = strtok(buffer, "|");
        if (!token) continue;
        
        MongoDocument* doc = &(*documents)[doc_count];
        memset(doc, 0, sizeof(MongoDocument));
        
        doc->id = atoi(token);
        
        token = strtok(NULL, "|");
        if (token) {
            doc->title = strdup(token);
        }
        
        token = strtok(NULL, "|");
        if (token) {
            doc->text = strdup(token);
        }
        
        token = strtok(NULL, "|");
        if (token) {
            doc->url = strdup(token);
        }
        
        token = strtok(NULL, "|");
        if (token) {
            char* newline = strchr(token, '\n');
            if (newline) *newline = '\0';
            doc->source = strdup(token);
        }
        
        doc_count++;
    }
    
    fclose(file);
    *count = doc_count;
    
    printf("Loaded %d documents from file\n", doc_count);
    return 1;
}

// Эта функция извлекает весь текст из HTML строки const char* html. В случае успеха возвращает указатель на полученную строку char*, а в случае неудачи - NULL
char* extract_text_from_html(const char* html) {
    if (!html) return NULL;
    
    char* text = (char*)malloc(strlen(html) + 1);
    if (!text) return NULL;
    
    int text_pos = 0;
    int in_tag = 0;
    int i = 0;
    
    while (html[i] && text_pos < strlen(html)) {
        if (html[i] == '<') {
            in_tag = 1;
        } else if (html[i] == '>') {
            in_tag = 0;
        } else if (!in_tag && !isspace(html[i])) {
            text[text_pos++] = tolower(html[i]);
        } else if (!in_tag && isspace(html[i]) && text_pos > 0 && text[text_pos-1] != ' ') {
            text[text_pos++] = ' ';
        }
        i++;
    }
    
    text[text_pos] = '\0';
    return text;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Эта функция разбивает текст const char* text на токены (слова), которые затем записываются в char** tokens. Лимит количества токенов указывается в int max_tokens. Возвращает количество записанных токенов.
int tokenize_mongo_document(const char* text, char** tokens, int max_tokens) {
    if (!text || !tokens || max_tokens <= 0) return 0;
    
    char* text_copy = strdup_custom(text);
    if (!text_copy) return 0;
    
    int token_count = 0;
    char* token = strtok(text_copy, " ,.!?;:\"'()[]{}<>-_\n\r\t");
    
    while (token && token_count < max_tokens) {
        // Фильтрация токенов
        int len = strlen(token);
        if (len >= 2 && len <= 50) {
            // Проверяем, что токен содержит только буквы и цифры
            /*
            int valid = 1;
            for (int i = 0; i < len; i++) {
                if (!isalnum(token[i])) {
                    valid = 0;
                    printf("invalid token:%s\n", token);
                    break;
                }
            }
            
            
            if (valid) {
                tokens[token_count] = strdup_custom(token);
                if (tokens[token_count]) {
                    token_count++;
                }
            }
            */
           tokens[token_count] = strdup_custom(token);
            if (tokens[token_count]) {
                token_count++;
            }
        }
        
        token = strtok(NULL, " ,.!?;:\"'()[]{}<>-_\n\r\t");
    }
    
    free(text_copy);
    return token_count;
}

// Эта функция индексации документов из файла с названием const char* filename. Она заполняет индекс BooleanIndex* index таким образом, чтобы он содержал все токены и id каждого документа из файла filename, где встречается токен. В случае успеха возвращает 1, а в случае неудачи - 0
int index_mongo_documents(BooleanIndex* index, const char* filename, int max_docs) {
    if (!index || !filename) return 0;
    
    printf("Loading documents from %s...\n", filename);
    
    MongoDocument* documents = NULL;
    int doc_count = 0;
    
    if (!load_documents_from_file(filename, &documents, &doc_count)) {
        printf("Failed to load documents\n");
        return 0;
    }
    
    printf("Indexing %d documents...\n", doc_count);
    
    int total_tokens = 0;
    int indexed_docs = 0;
    
    for (int i = 0; (i < doc_count) && (i < max_docs); i++) {
        MongoDocument* doc = &documents[i];
        
        // Извлекаем текст из HTML
        char* text = extract_text_from_html(doc->text);
        if (!text) continue;
        
        // Токенизация
        char* tokens[1000];
        int token_count = tokenize_mongo_document(text, tokens, 1000);
        
        // Добавляем термины в индекс
        for (int j = 0; j < token_count; j++) {
            if (add_term(index, tokens[j], doc->id)) {
                total_tokens++;
            }
            free(tokens[j]);
        }
        
        free(text);
        indexed_docs++;
        
        if (i % 100 == 0) {
            printf("Indexed %d/%d documents (%d tokens)\n", i, doc_count, total_tokens);
        }
    }
    
    printf("Indexing completed: %d documents, %d tokens\n", indexed_docs, total_tokens);
    
    cleanup_mongo_documents(documents, doc_count);
    return 1;
}

// Эта функция печатает содержимое структуры MongoDocument* doc
void print_mongo_document(MongoDocument* doc) {
    if (!doc) return;
    
    printf("Document ID: %d\n", doc->id);
    printf("Title: %s\n", doc->title);
    printf("Source: %s\n", doc->source);
    printf("URL: %s\n", doc->url);
    printf("---\n");
}

// Эта функция уничтожает int count объектов структур из их списка MongoDocument* documents
void cleanup_mongo_documents(MongoDocument* documents, int count) {
    if (documents) {
        for (int i = 0; i < count; i++) {
            if (documents[i].title) free(documents[i].title);
            if (documents[i].text) free(documents[i].text);
            if (documents[i].url) free(documents[i].url);
            if (documents[i].source) free(documents[i].source);
        }
        free(documents);
    }
}

// Эта функция создает файл данных для тестирования функций обработки документов. В случае успешного создания возвращает 1, а в случае неудачи - 0.
int create_mongo_export_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return 0;
    
    // Тестовые данные в формате MongoDB
    fprintf(file, "0|Комплекс мертвой матери|это психологический термин, введеный французским психоаналитиком Андре Грином|https://ru.wikipedia.org/wiki/%%D0%%9A%%D0%%BE%%D0%%BC%%D0%%BF%%D0%%BB%%D0%%B5%D0%%BA%%D1%%81_%%D0%%BC%%D0%%B5%%D1%%80%%D1%%82%%D0%%B2%%D0%%BE%%D0%%B9_%%D0%%BC%%D0%%B0%%D1%%82%%D0%%B5%%D1%%80%%D0%%B8|wikipedia\n");
    fprintf(file, "1|Поведенческая иммунная система|набор психологических и поведенческих механизмов, позволяющих человеку, с одной стороны, опознать инфекции, вредные вещества или вызывающих болезни паразитов|https://ru.wikipedia.org/wiki/%%D0%%9F%%D0%%BE%%D0%%B2%%D0%%B5%%D0%%B4%%D0%%B5%%D0%%BD%%D1%%87%%D0%%B5%%D1%%81%%D0%%BA%%D0%%B0%%D1%%8F_%%D0%%B8%%D0%%BC%%D0%%BC%%D1%%83%%D0%%BD%%D0%%BD%%D0%%B0%%D1%%8F_%%D1%%81%%D0%%B8%%D1%%81%%D1%%82%%D0%%B5%%D0%%BC%%D0%%B0|wikipedia\n");
    fprintf(file, "2|Саморефлексия|способность человека погружаться в индивидуальный внутренний мир психики, содержащий чувства, мысли, состояния|https://ru.wikipedia.org/wiki/%%D0%%A1%%D0%%B0%%D0%%BC%%D0%%BE%%D1%%80%%D0%%B5%%D1%%84%%D0%%BB%%D0%%B5%%D0%%BA%%D1%%81%%D0%%B8%%D1%%8F|wikipedia\n");
    
    fclose(file);
    printf("Created test export file: %s\n", filename);
    return 1;
}