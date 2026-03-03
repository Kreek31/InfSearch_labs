import pymongo
from bs4 import BeautifulSoup
import re

#TODO: подключить использование config.json

def connect_to_mongodb():
    """Подключение к MongoDB с данными от parser.py"""
    try:
        client = pymongo.MongoClient("localhost", 27017)
        db = client["psychology_search"]
        collection = db["documents"]
        doc_count = collection.count_documents({})
        print(f"Connected to MongoDB, found {doc_count} documents from parser")
        return collection
    except Exception as e:
        print(f"Error connecting to MongoDB: {e}")
        return None

def clean_text(text):
    """Очистка текста от HTML тегов и спецсимволов"""
    if not text:
        return ""
    
    soup = BeautifulSoup(text, 'html.parser')
    text = soup.get_text()
    
    text = re.sub(r'\s+', ' ', text).strip()
    
    text = re.sub(r'[^\w\s]', '', text)
    
    if len(text) > 1000:
        text = text[:1000]
    
    return text

def export_to_file(collection, filename="mongo_export.txt"):
    """Экспорт документов из parser.py в файл для программ на языке C"""
    try:
        documents = collection.find({})
        total_docs = collection.count_documents({})
        
        print(f"Starting export of {total_docs} documents...")
        
        with open(filename, 'w', encoding='utf-8') as f:
            exported_count = 0
            
            for i, doc in enumerate(documents):
                # поле id
                doc_id = doc.get('_id', i)

                # поле title
                title = clean_text(doc.get('title', ''))
                
                # поле raw_html
                raw_html = doc.get('raw_html', '')
                if raw_html:
                    text = clean_text(raw_html)
                
                # поле url
                url = doc.get('url', '')
                
                # Пропускаем документы без текста
                if not text or len(text) < 10:
                    continue
                
                # Формат: doc_id|title|text|url|source
                line = f"{doc_id}|{title}|{text}|{url}|wikipedia\n"
                f.write(line)
                exported_count += 1
                
                if exported_count % 1000 == 0:
                    print(f"Exported {exported_count}/{total_docs} documents")
        
        print(f"Successfully exported {exported_count} documents to {filename}")
        return True
        
    except Exception as e:
        print(f"Error exporting documents: {e}")
        return False

def create_sample_data(filename="mongo_export.txt"):
    """Создание примера данных если MongoDB недоступна"""
    sample_docs = [
        {
            'doc_id': 0,
            'title': 'Комплекс мертвой матери',
            'text': 'психологический термин, введеный французским психоаналитиком Андре Грином',
            'url': 'https://ru.wikipedia.org/wiki/%D0%9A%D0%BE%D0%BC%D0%BF%D0%BB%D0%B5%D0%BA%D1%81_%D0%BC%D0%B5%D1%80%D1%82%D0%B2%D0%BE%D0%B9_%D0%BC%D0%B0%D1%82%D0%B5%D1%80%D0%B8',
            'source': 'wikipedia'
        },
        {
            'doc_id': 1,
            'title': 'Поведенческая иммунная система',
            'text': 'набор психологических и поведенческих механизмов, позволяющих человеку, с одной стороны, опознать инфекции, вредные вещества или вызывающих болезни паразитов',
            'url': 'https://ru.wikipedia.org/wiki/%D0%9F%D0%BE%D0%B2%D0%B5%D0%B4%D0%B5%D0%BD%D1%87%D0%B5%D1%81%D0%BA%D0%B0%D1%8F_%D0%B8%D0%BC%D0%BC%D1%83%D0%BD%D0%BD%D0%B0%D1%8F_%D1%81%D0%B8%D1%81%D1%82%D0%B5%D0%BC%D0%B0',
            'source': 'wikipedia'
        },
        {
            'doc_id': 2,
            'title': 'Саморефлексия',
            'text': 'способность человека погружаться в индивидуальный внутренний мир психики, содержащий чувства, мысли, состояния',
            'url': 'https://ru.wikipedia.org/wiki/%D0%A1%D0%B0%D0%BC%D0%BE%D1%80%D0%B5%D1%84%D0%BB%D0%B5%D0%BA%D1%81%D0%B8%D1%8F',
            'source': 'wikipedia'
        }
    ]
    
    try:
        with open(filename, 'w', encoding='utf-8') as f:
            for doc in sample_docs:
                line = f"{doc['doc_id']}|{doc['title']}|{doc['text']}|{doc['url']}|{doc['source']}\n"
                f.write(line)
        
        print(f"Created sample data with {len(sample_docs)} documents in {filename}")
        return True
        
    except Exception as e:
        print(f"Error creating sample data: {e}")
        return False

def main():
    print("MongoDB Export for Boolean Index")
    print("================================")
    
    filename = "mongo_export.txt"
    
    # Пробуем подключиться к MongoDB
    collection = connect_to_mongodb()
    
    if collection is not None:
        # Экспортируем реальные данные
        success = export_to_file(collection, filename)
        if success:
            print(f"Data exported to {filename}")
            print("You can now run: gcc -o mongo_search boolean_index.c boolean_search.c mongo_loader.c main_mongo.c")
    else:
        # Создаем пример данных
        print("MongoDB not available, creating sample data...")
        success = create_sample_data(filename)
        if success:
            print(f"Sample data created in {filename}")
            print("You can now run: gcc -o mongo_search boolean_index.c boolean_search.c mongo_loader.c main_mongo.c")
    
    # Инструкции
    print("\nInstructions:")
    print("1. Make sure MongoDB is running and has data")
    print("2. Run this script to export data")
    print("3. Compile and run the C/C++ program:")
    print("   cd cpp")
    print("   gcc -o mongo_search boolean_index_functions.c boolean_search_functions.c document_functions.c main.c")
    print("   ./mongo_search")

if __name__ == "__main__":
    main()