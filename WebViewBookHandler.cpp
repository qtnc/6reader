#include "WebViewBookHandler.hpp"
#include "Book.hpp"
#include "cpprintf.hpp"
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/filesys.h>
#include "stringUtils.hpp"
using namespace std;

WebViewBookHandler::WebViewBookHandler ():
wxWebViewHandler("book") {}

void WebViewBookHandler::addBook (const std::shared_ptr<Book>& b) {
println("Adding book to FSHandler: %s", b->getFileName());
books[b->getFileName()] = b;
}

void WebViewBookHandler::removeBook (const std::shared_ptr<Book>& b) {
if (!b) return;
println("Removing book from FSHandler: %s", b->getFileName());
auto filename = b->getFileName();
auto itb = books.find(filename);
if (itb!=books.end()) books.erase(itb);
for (auto it = cache.begin(); it!=cache.end(); ) {
if (starts_with(it->first, filename)) {
cacheSize -= it->second.first.size();
it = cache.erase(it);
}
else ++it;
}
}

void WebViewBookHandler::drainCache () {
if (cacheSize < maxCacheSize) return;
cache.clear();
}

wxFSFile* WebViewBookHandler::GetFile (const wxString& wxuri) {
auto uri = urldecode(U(wxuri.substr(5)));
auto idx = uri.find("!/");
if (idx==string::npos) return nullptr;
auto left = uri.substr(0, idx);
auto right = uri.substr(idx+2);
auto book = books[left];
if (!book) return nullptr;
auto resource = book->findResourceByURI(right);
if (!resource) return nullptr;
drainCache();
auto& content = cache[uri];
if (content.first.empty()) {
auto ff = book->openResource(resource);
if (!ff || !ff->IsOk()) return nullptr;
content.first = ReadAll(*ff);
}
content.second++;
auto stream = new wxMemoryInputStream(content.first.data(), content.first.size());
return new wxFSFile(stream, uri, U(resource->type), wxEmptyString, wxDateTime());
}
