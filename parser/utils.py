from bs4 import BeautifulSoup
from urllib.parse import urlparse, urlunparse

def get_links(html):
    soup = BeautifulSoup(html, "html.parser")
    links = set()
    for i in soup.find_all("a", href=True):
        href = i["href"]
        if href.startswith("/wiki/") and ":" not in href:
            links.add("https://ru.wikipedia.org" + href)
    return links

def normalize_url(url: str) -> str:
    parsed = urlparse(url)
    return urlunparse((
        parsed.scheme.lower(),
        parsed.netloc.lower(),
        parsed.path.rstrip('/'),
        '', 
        '', 
        ''
    ))