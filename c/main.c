#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <time.h>
#include "boolean_index_functions.h"
#include "boolean_search_functions.h"
#include "document_functions.h"


// Эта функция демонстрирует работу булева поиска в структуре BooleanSearch* search_obj по документам MongoDocument* documents.
void demonstrate_mongo_search(BooleanSearch* search_obj, MongoDocument* documents, int doc_count) {
    printf("\n=== MongoDB Search Demonstration ===\n\n");
    
    FILE* output_file = fopen("search_results.txt", "w");
    if (output_file) {
        fprintf(output_file, "Boolean Search Results\n");
        fprintf(output_file, "====================\n\n");
    }

    int num_queries = 8;
    
    const char* queries[8] = {
        "психология",
        "терапия", 
        "состояние",
        "психология AND терапия",
        "психология OR терапия",
        "психология NOT терапия",
        "стабильность AND состояние",
        "здоровый OR стабильный",
    };
    
    for (int i = 0; i < num_queries; i++) {
        printf("Query %d: \"%s\"\n", i + 1, queries[i]);
        
        if (output_file) {
            fprintf(output_file, "Query %d: \"%s\"\n", i + 1, queries[i]);
        }
        
        SearchResult* result = search(search_obj, queries[i]);
        if (result && result->size > 0) {
            printf("Results (%d documents):\n", result->size);
            if (output_file) {
                fprintf(output_file, "Results (%d documents):\n", result->size);
            }
            
            for (int j = 0; j < result->size; j++) {
                int doc_id = result->doc_ids[j];
                // Ищем документ по id
                for (int k = 0; k < doc_count; k++) {
                    if (documents[k].id == doc_id) {
                        //Печатаем id, название и url найденного документа
                        printf("  %d: %s (%s)\n", doc_id, documents[k].title, documents[k].url);
                        if (output_file) {
                            fprintf(output_file, "  %d: %s (%s)\n", doc_id, documents[k].title, documents[k].url);
                        }
                        break;
                    }
                }
            }
            printf("\n");
            if (output_file) {
                fprintf(output_file, "\n");
            }
        } else {
            printf("No results found.\n\n");
            if (output_file) {
                fprintf(output_file, "No results found.\n\n");
            }
        }
        
        if (result) {
            free(result->doc_ids);
            free(result);
            result = NULL;
        }
    }
    
    if (output_file) {
        fclose(output_file);
        printf("Results also saved to 'search_results.txt' file\n");
    }
}

// Эта функция выводит статистику индекса BooleanIndex* index
void analyze_mongo_index(BooleanIndex* index) {
    printf("\n=== MongoDB Index Analysis ===\n\n");
    
    print_index_stats(index);
    
    // Показываем топ термины
    printf("\nTop terms by document frequency:\n");
    
    // Находим термины с наибольшей частотой
    int max_freq = 0;
    int total_terms = index->size;
    
    for (int i = 0; i < total_terms && i < 20; i++) {
        int current_max = 0;
        int max_index = -1;
        
        for (int j = 0; j < total_terms; j++) {
            if (index->terms[j].doc_frequency > current_max) {
                current_max = index->terms[j].doc_frequency;
                max_index = j;
            }
        }
        
        if (max_index >= 0 && current_max > 0) {
            printf("  %s: %d documents\n", index->terms[max_index].term, current_max);
            index->terms[max_index].doc_frequency *= -1; // Временно помечаем как использованный
        }
    }
    
    // Восстанавливаем частоты
    for (int i = 0; i < total_terms; i++) {
        if (index->terms[i].doc_frequency < 0) {
            index->terms[i].doc_frequency *= -1;
        }
    }
}

// Эта функция тестирования скорости выполнения поиска
void test_performance(BooleanSearch* search_obj) {
    printf("\n=== Performance Test ===\n\n");
    
    int num_queries = 5;
    const char* test_queries[5] = {
        "психика",
        "психология",
        "состояние",
        "человек AND состояние",
        "психика OR психология"
    };

    
    printf("Testing %d queries...\n", num_queries);
    
    for (int i = 0; i < num_queries; i++) {
        clock_t start = clock();
        
        SearchResult* result = search(search_obj, test_queries[i]);
        
        clock_t end = clock();
        double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC * 1000; // в мс
        
        printf("Query \"%s\": %.3f ms, %d results\n", 
               test_queries[i], time_spent, result ? result->size : 0);
        
        if (result) {
            destroy_search_result(result);
        }
    }
}


void tokens_to_file(BooleanIndex* index){
    printf("\nLoading tokens statistics to file cipf.csv\n");
    FILE* f = fopen("cipf.csv", "w");
    if (!f) {
        perror("Cannot open file");
        return;
    }

    // Заголовок
    fprintf(f, "rank,frequency,term\n");

    // Находим термины с наибольшей частотой
    int max_freq = 0;
    int total_terms = index->size;
    
    for (int i = 0; i < total_terms; i++) {
        int current_max = 0;
        int max_index = -1;
        
        for (int j = 0; j < total_terms; j++) {
            if (index->terms[j].doc_frequency > current_max) {
                current_max = index->terms[j].doc_frequency;
                max_index = j;
            }
        }
        
        if (max_index >= 0 && current_max > 0) {
            //printf("  %s: %d documents\n", index->terms[max_index].term, current_max);
            fprintf(f, "%d,%d,%s\n", i+1, current_max, index->terms[max_index].term);
            index->terms[max_index].doc_frequency *= -1; // Временно помечаем как использованный
        }
    }
    
    // Восстанавливаем частоты
    for (int i = 0; i < total_terms; i++) {
        if (index->terms[i].doc_frequency < 0) {
            index->terms[i].doc_frequency *= -1;
        }
    }

    fclose(f);
    printf("CSV file written successfully.\n");
}

int main(int argc, char* argv[]) {
    // Устанавливаем локаль
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".65001");
    
    printf("Boolean Search System with MongoDB Integration\n");
    printf("==============================================\n\n");
    
    // Имя файла с данными MongoDB
    const char* data_file = "mongo_export.txt";
    
    // Создаем тестовый файл если не существует
    FILE* test_file = fopen(data_file, "r");
    if (!test_file) {
        // Убираем вызов несуществующей функции
        printf("File 'test_mongo_data.txt' not found. Please run 'python export_mongo.py' first to create data file\n");
        return 1;
    } else {
        fclose(test_file);
    }
    
    // Создаем индекс
    BooleanIndex* index = create_boolean_index();
    if (!index) {
        printf("Failed to create boolean index\n");
        return 1;
    }
    
    // Индексируем документы из MongoDB
    int max_docs = 10000;
    printf("Индексируем документы из mongoDB. Максимальное количество индексируемых документов=%d\n", max_docs);
    if (!index_mongo_documents(index, data_file, max_docs)) {
        printf("Failed to index MongoDB documents\n");
        destroy_boolean_index(index);
        return 1;
    }
    
    // Анализируем индекс
    analyze_mongo_index(index);
    
    // Создаем поисковую систему
    BooleanSearch* search = create_boolean_search(index);
    if (!search) {
        printf("Failed to create boolean search\n");
        destroy_boolean_index(index);
        return 1;
    }
    
    // Загружаем метаданные документов
    MongoDocument* documents = NULL;
    int doc_count = 0;
    if (!load_documents_from_mongo(&documents, &doc_count)) {
        printf("Failed to load MongoDB documents metadata\n");
        destroy_boolean_index(index);
        return 1;
    }
    
    printf("Loaded %d documents from %s\n", doc_count, data_file);
    
    // Демонстрация поиска
    demonstrate_mongo_search(search, documents, doc_count);
    
    // Тестирование производительности
    test_performance(search);

    // Сохранение частот токенов в файл 
    tokens_to_file(index);
    
    // Сохранение индекса
    printf("\nSaving index to file...\n");
    if (save_index_to_file(index, "mongo_boolean_index.bin")) {
        printf("MongoDB index saved successfully\n");
    } else {
        printf("Failed to save index\n");
    }
    
    // Тест загрузки
    printf("\nTesting index loading...\n");
    BooleanIndex* loaded_index = create_boolean_index();
    if (loaded_index) {
        if (load_index_from_file(loaded_index, "mongo_boolean_index.bin")) {
            printf("MongoDB index loaded successfully\n");
            print_index_stats(loaded_index);
        } else {
            printf("Failed to load index\n");
        }
        destroy_boolean_index(loaded_index);
    }
    
    // Очистка
    cleanup_mongo_documents(documents, doc_count);
    destroy_boolean_search(search);
    destroy_boolean_index(index);
    
    printf("\nMongoDB integration completed successfully.\n");
    printf("Use the exported file '%s' from your real MongoDB database\n", data_file);
    printf("Format: doc_id|title|text|url|source\n");
    
    return 0;
}