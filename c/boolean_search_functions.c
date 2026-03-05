#include "boolean_search_functions.h"
#include <ctype.h>

//TODO: строка 198: PostingList* execute_query(BooleanSearch* search, BooleanQueryNode* node) - перепроверить необходимость использования структуры BooleanSearch вместо BooleanIndex, добавить обработку случаев, когда терм отустствует в структуре BooleanIndex и когда дерево запроса содержит несколько операторов.



// Эта функция создает узел булева запроса, содержащего терм const char* term. В случае успеха возвращает указатель на структуру BooleanQueryNode, а в случае неудачи NULL
BooleanQueryNode* create_term_node(const char* term) {
    BooleanQueryNode* node = malloc(sizeof(BooleanQueryNode));
    if (!node) return NULL;
    
    node->op = OP_TERM;
    strncpy(node->term, term, MAX_TERM_LENGTH - 1);
    node->term[MAX_TERM_LENGTH - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    node->result = NULL;
    
    return node;
}

// Эта функция создает узел булева запроса, содержащего оператор BooleanOperator op и располагающегося между BooleanQueryNode* left и BooleanQueryNode* right. В случае успеха возвращает указатель на структуру BooleanQueryNode, а в случае неудачи NULL
BooleanQueryNode* create_operator_node(BooleanOperator op, BooleanQueryNode* left, BooleanQueryNode* right) {
    BooleanQueryNode* node = malloc(sizeof(BooleanQueryNode));
    if (!node) return NULL;
    
    node->op = op;
    node->term[0] = '\0';
    node->left = left;
    node->right = right;
    node->result = NULL;
    
    return node;
}

// Эта функция уничтожает дерево структур запроса, содержащее BooleanQueryNode* node
void destroy_query_tree(BooleanQueryNode* node) {
    if (!node) return;
    
    destroy_query_tree(node->left);
    destroy_query_tree(node->right);
    
    if (node->result) {
        destroy_posting_list(node->result);
    }
    
    free(node);
}

// Эта функция создания структуры булева поиска для индекса BooleanIndex* index. В случае успеха возвращает указатель на структуру BooleanSearch, а в случае неудачи NULL
BooleanSearch* create_boolean_search(BooleanIndex* index) {
    BooleanSearch* search = malloc(sizeof(BooleanSearch));
    if (!search) return NULL;
    
    search->index = index;
    search->query_tree = NULL;
    
    return search;
}

// Эта функция уничтожения структуры булева поиска
void destroy_boolean_search(BooleanSearch* search) {
    if (!search) return;
    
    destroy_query_tree(search->query_tree);
    free(search);
}

// Эта функция ищет пересечение (логическое и) структур PostingList* list1 и PostingList* list2. В случае успеха возвращает указатель на структуру PostingList, содержащую все найденные пересечения, а в случае неудачи или отсутствия пересечений - NULL
PostingList* intersect_postings(PostingList* list1, PostingList* list2) {
    if (!list1 || !list2) return NULL;
    if (list1->size == 0 || list2->size == 0) return NULL;
    
    PostingList* result = create_posting_list();
    if (!result) return NULL;
    
    int i = 0, j = 0;
    
    while (i < list1->size && j < list2->size) {
        if (list1->doc_ids[i] == list2->doc_ids[j]) {
            add_doc_to_posting(result, list1->doc_ids[i]);
            i++;
            j++;
        } else if (list1->doc_ids[i] < list2->doc_ids[j]) {
            i++;
        } else {
            j++;
        }
    }
    
    return result;
}

// Эта функция ищет объединение (логическое или) структур PostingList* list1 и PostingList* list2. В случае успеха возвращает указатель на структуру PostingList, содержащую все найденные элементы, а в случае неудачи или отсутствия элементов - NULL
PostingList* union_postings(PostingList* list1, PostingList* list2) {
    if (!list1 && !list2) return NULL;
    if (!list1) return create_posting_list_copy(list2);
    if (!list2) return create_posting_list_copy(list1);
    
    PostingList* result = create_posting_list();
    if (!result) return NULL;
    
    int i = 0, j = 0;
    
    while (i < list1->size && j < list2->size) {
        if (list1->doc_ids[i] == list2->doc_ids[j]) {
            add_doc_to_posting(result, list1->doc_ids[i]);
            i++;
            j++;
        } else if (list1->doc_ids[i] < list2->doc_ids[j]) {
            add_doc_to_posting(result, list1->doc_ids[i]);
            i++;
        } else {
            add_doc_to_posting(result, list2->doc_ids[j]);
            j++;
        }
    }
    
    // Добавляем оставшиеся элементы
    while (i < list1->size) {
        add_doc_to_posting(result, list1->doc_ids[i]);
        i++;
    }
    
    while (j < list2->size) {
        add_doc_to_posting(result, list2->doc_ids[j]);
        j++;
    }
    
    return result;
}

// Эта функция ищет разность (логическое не) структур PostingList* list1 и PostingList* list2 (элементы, присутстсвующие в PostingList* list1, но отсутствующие в PostingList* list2). В случае успеха возвращает указатель на структуру PostingList, содержащую все найденные элементы, а в случае неудачи или отсутствия элементов - NULL
PostingList* difference_postings(PostingList* list1, PostingList* list2) {
    if (!list1) return NULL;
    if (!list2) return create_posting_list_copy(list1);
    
    PostingList* result = create_posting_list();
    if (!result) return NULL;
    
    int i = 0, j = 0;
    
    while (i < list1->size && j < list2->size) {
        if (list1->doc_ids[i] == list2->doc_ids[j]) {
            i++;
            j++;
        } else if (list1->doc_ids[i] < list2->doc_ids[j]) {
            add_doc_to_posting(result, list1->doc_ids[i]);
            i++;
        } else {
            j++;
        }
    }
    
    // Добавляем оставшиеся элементы из list1
    while (i < list1->size) {
        add_doc_to_posting(result, list1->doc_ids[i]);
        i++;
    }
    
    return result;
}

// Эта функция создает копию структуры PostingList* original. В случае успеха возвращает указатель на копию PostingList, а в случае неудачи - NULL
PostingList* create_posting_list_copy(PostingList* original) {
    if (!original) return NULL;
    
    PostingList* copy = create_posting_list();
    if (!copy) return NULL;
    
    // Расширяем если необходимо
    if (original->size > copy->capacity) {
        copy->capacity = original->size;
        copy->doc_ids = (int*)realloc(copy->doc_ids, copy->capacity * sizeof(int));
        if (!copy->doc_ids) {
            destroy_posting_list(copy);
            return NULL;
        }
    }
    
    // Копируем данные
    copy->size = original->size;
    for (int i = 0; i < original->size; i++) {
        copy->doc_ids[i] = original->doc_ids[i];
    }
    
    return copy;
}

/* Эта функция выполнения запроса булева поиска. Указываются параметры:

    BooleanSearch* search - структура, содержащая BoleanIndex, в которой хранятся термы и сопоставленные им id документов
    BooleanQueryNode* node - указатель на главный узел (оператор) дерева булева запроса. На данный момент поддерживаются деревья вида: а)term-OP-term и б)term

В случае успеха возвращает указатель на структуру PostingList со списком найденных документов, а в случае неудачи - NULL.
*/
PostingList* execute_query(BooleanSearch* search, BooleanQueryNode* node) {
    if (!search || !node) return NULL;
    
    // Если результат уже вычислен, возвращаем его
    if (node->result) {
        return node->result;
    }
    
    switch (node->op) {
        case OP_TERM: {
            // Получаем posting list для терма (ВАЖНО, терм должен быть в списке search->index)
            node->result = get_posting_list(search->index, node->term);
            if (node->result) {
                node->result = create_posting_list_copy(node->result);
            }
            return node->result;
        }
        
        case OP_AND: {
            PostingList* left_result = execute_query(search, node->left);
            PostingList* right_result = execute_query(search, node->right);
            node->result = intersect_postings(left_result, right_result);
            if (!(node->result)) return NULL;
            
            /*Результаты потом будут очищены в destroy_query_tree()
            // Очищаем временные результаты
            if (left_result && left_result != node->result) {
                destroy_posting_list(left_result);
            }
            if (right_result && right_result != node->result) {
                destroy_posting_list(right_result);
            }
            */

            return node->result;
        }
        
        case OP_OR: {
            PostingList* left_result = execute_query(search, node->left);
            PostingList* right_result = execute_query(search, node->right);
            node->result = union_postings(left_result, right_result);
            
            /*Результаты потом будут очищены в destroy_query_tree()
            // Очищаем временные результаты
            if (left_result && left_result != node->result) {
                destroy_posting_list(left_result);
            }
            if (right_result && right_result != node->result) {
                destroy_posting_list(right_result);
            }
            */
            return node->result;
        }
        
        case OP_NOT: {
            PostingList* left_result = execute_query(search, node->left);
            PostingList* right_result = execute_query(search, node->right);
            node->result = difference_postings(left_result, right_result);
            
            /*Результаты потом будут очищены в destroy_query_tree()
            // Очищаем временные результаты
            if (left_result && left_result != node->result) {
                destroy_posting_list(left_result);
            }
            if (right_result && right_result != node->result) {
                destroy_posting_list(right_result);
            }
            */
            return node->result;
        }
        
        default:
            return NULL;
    }
}

// Эта функция проверяет, является ли токен const char* token оператором. Возвращает 1, если является и 0, если нет.
int is_operator_token(const char* token) {
    if (!token) return 0;
    
    return (strcmp(token, "AND") == 0 || 
            strcmp(token, "OR") == 0 || 
            strcmp(token, "NOT") == 0 ||
            strcmp(token, "and") == 0 || 
            strcmp(token, "or") == 0 || 
            strcmp(token, "not") == 0);
}

// Эта функция получает на основе токена const char* token значение оператора из перечисления BooleanOperator. В случае успеха возвращает значение перечисления BooleanOperator, а в случае неудачи - BooleanOperator=OP_TERM
BooleanOperator get_operator(const char* token) {
    if (!token) return OP_TERM;
    
    if (strcmp(token, "AND") == 0 || strcmp(token, "and") == 0) {
        return OP_AND;
    } else if (strcmp(token, "OR") == 0 || strcmp(token, "or") == 0) {
        return OP_OR;
    } else if (strcmp(token, "NOT") == 0 || strcmp(token, "not") == 0) {
        return OP_NOT;
    }
    
    return OP_TERM;
}

// Эта функция разбирает строку const char* query_str на дерево булева запроса (инфиксная нотация). В случае успеха возвращает указатель на узел дерева BooleanQueryNode, а в случае неудачи - NULL
BooleanQueryNode* parse_query_simple(const char* query_str) {
    if (!query_str) return NULL;
    
    // Копируем строку для обработки
    char* query_copy = strdup_custom(query_str);
    if (!query_copy) return NULL;
    
    // Простая реализация: поддерживаем только термины и операторы AND/OR
    char* token = strtok(query_copy, " ");
    BooleanQueryNode* root = NULL;
    BooleanQueryNode* current = NULL;
    BooleanOperator last_op = OP_OR; // По умолчанию OR
    
    while (token) {
        // Удаляем лишние пробелы и приводим к нижнему регистру
        char* clean_token = token;
        while (*clean_token && isspace(*clean_token)) clean_token++;
        
        if (strlen(clean_token) == 0) {
            token = strtok(NULL, " ");
            continue;
        }
        
        if (is_operator_token(clean_token)) {
            last_op = get_operator(clean_token);
        } else {
            // Это терм
            BooleanQueryNode* term_node = create_term_node(clean_token);
            
            if (!root) {
                root = term_node;
                current = term_node;
            } else {
                // Создаем операторный узел
                BooleanQueryNode* op_node = create_operator_node(last_op, current, term_node);
                if (!op_node) {
                    destroy_query_tree(root);
                    free(query_copy);
                    return NULL;
                }
                
                if (root == current) {
                    root = op_node;
                }
                current = op_node;
            }
        }
        
        token = strtok(NULL, " ");
    }
    
    free(query_copy);
    //printf("node term:%s\n", root->term);
    return root;
}

// Эта функция выполняет булевый поиск внутри структуры BooleanSearch* search по строке запроса const char* query_str. В случае успеха возвращает указатель на структуру SearchResult, а в случае неудачи - NULL
SearchResult* search(BooleanSearch* search, const char* query_str) {
    if (!search || !query_str) return NULL;
    
    // Парсим запрос
    BooleanQueryNode* query_tree = parse_query_simple(query_str);
    if (!query_tree) return NULL;
    
    // Выполняем запрос
    PostingList* postings = execute_query(search, query_tree);
    if (!postings){
        destroy_query_tree(query_tree);
        return NULL;
    }
    
    // Создаем результат
    SearchResult* result = malloc(sizeof(SearchResult));
    if (!result) {
        destroy_query_tree(query_tree);
        return NULL;
    }
    
    if (postings) {
        result->size = postings->size;
        result->capacity = postings->size;
        result->doc_ids = (int*)malloc(result->capacity * sizeof(int));
        
        if (result->doc_ids) {
            for (int i = 0; i < postings->size; i++) {
                result->doc_ids[i] = postings->doc_ids[i];
            }
        } else {
            result->size = 0;
            result->capacity = 0;
        }
    } else {
        result->size = 0;
        result->capacity = 0;
        result->doc_ids = NULL;
    }
    
    destroy_query_tree(query_tree);
    return result;
}

// Эта функция уничтожения результата поиска SearchResult* result
void destroy_search_result(SearchResult* result) {
    if (!result) return;
    
    if (result->doc_ids) {
        free(result->doc_ids);
    }
    free(result);
}

// Эта функция печатает содержимое структуры SearchResult* result
void print_search_result(SearchResult* result) {
    if (!result) {
        printf("Search result is NULL\n");
        return;
    }
    
    printf("Search Results (%d documents):\n", result->size);
    printf("[");
    for (int i = 0; i < result->size; i++) {
        printf("%d", result->doc_ids[i]);
        if (i < result->size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

// Эта функция печатает дерево запроса, включающее в себя узел BooleanQueryNode* node
void print_query_tree(BooleanQueryNode* node, int depth) {
    if (!node) return;
    
    // Отступ
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    switch (node->op) {
        case OP_TERM:
            printf("TERM: %s\n", node->term);
            break;
        case OP_AND:
            printf("AND\n");
            break;
        case OP_OR:
            printf("OR\n");
            break;
        case OP_NOT:
            printf("NOT\n");
            break;
    }
    
    print_query_tree(node->left, depth + 1);
    print_query_tree(node->right, depth + 1);
}