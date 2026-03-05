import json
import time
from bs4 import BeautifulSoup
import hashlib
import requests
from pymongo import MongoClient
from utils import normalize_url
from utils import get_links
from collections import deque

with open("config.json", "r", encoding="utf-8") as f:
    config = json.load(f)

db_conf = config["db"]
logic = config["logic"]

client = MongoClient(db_conf["host"], db_conf["port"])
db = client[db_conf["database"]]
collection = db[db_conf["collection"]]

headers = {"User-Agent": logic["user_agent"]}
doc_id = 0


def rebuild_queue_from_db():
    visited = set()
    queue = []

    cursor = collection.find({}, {"normalized_url": 1, "raw_html": 1})

    for doc in cursor:
        if doc["_id"] > doc_id:
            doc_id = doc["_id"] + 1

        visited.add(doc["normalized_url"])
        links = get_links(doc["raw_html"])
        for link in links:
            norm = normalize_url(link)
            if norm not in visited:
                queue.append((link))
    return queue, visited

def get_hash(html: str) -> str:
    return hashlib.md5(html.encode("utf-8")).hexdigest()

def parse():
    queue, visited = rebuild_queue_from_db()
    
    for src in config["sources"]:
        queue.append(src["start_url"])

    while queue and (collection.count_documents({}) < logic["max_pages"]):
        url = queue.pop(0)
        norm_url = normalize_url(url)

        if norm_url in visited:
            continue

        try:
            response = requests.get(url, headers=headers, timeout=10)
            html_code = response.text
            soup = BeautifulSoup(html_code, "html.parser")
            title = soup.title.string.strip() if soup.title else ""

            html_hash = get_hash(html_code)
            existing = collection.find_one({"normalized_url": norm_url})

            if existing and existing.get("html_hash") == html_hash:
                continue

            collection.update_one(
                {"normalized_url": norm_url},
                {"$set": {
                    "_id": doc_id,
                    "url": url,
                    "normalized_url": norm_url,
                    "title": title,
                    "raw_html": html_code,
                    "html_hash": html_hash,
                    "timestamp": int(time.time()),
                    }
                },
                upsert=True
            )
            doc_id += 1

            visited.add(norm_url)

            links = get_links(html_code)
            for link in links:
                queue.append((link))

            time.sleep(0.1)
            print("Parsed. URL:", url)

        except Exception as e:
            print("Error. URL:", url, "\nexception:", e)

if __name__ == "__main__":
    parse()