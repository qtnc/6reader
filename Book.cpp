#include "App.hpp"
#include "Book.hpp"
#include "BookEpub.hpp"
#include "BookCHM.hpp"
#include "BookHTML.hpp"
#include "BookTXT.hpp"
#include "BookDIC.hpp"
#include "stringUtils.hpp"
#include "cpprintf.hpp"
using namespace std;

decltype(Book::loaders) Book::loaders;
decltype(Book::filenameFilters) Book::filenameFilters;

void Book::init () {
loaders.clear();
loaders.push_back(&BookEpub::open);
loaders.push_back(&BookCHM::open);
loaders.push_back(&BookHTML::open);
loaders.push_back(&BookDIC::open);
loaders.push_back(BookTXT::open);

filenameFilters = {
{ "E-Pub book 2.0 / 3.0", "*.epub" },
{ "Compressed help Microsoft", "*.chm" },
{ "HTML, HTML5, HTML4", "*.html;*.htm" },
{ "Text plain", "*.txt" }
};
}

shared_ptr<Book> Book::open (const std::string& filename) {
shared_ptr<Book> book = nullptr;
if (loaders.empty()) init();
for (auto& loader: loaders) if (book=loader(filename)) break;
return book;
}

std::string Book::getHashName () {
string filename = this->filename;
if (ends_with(filename, "/") || ends_with(filename, "\\")) filename=filename.substr(0, filename.size() -1);
auto i1 = filename.rfind('/'), i2 = filename.rfind('\\'), i3 = std::min(i1, i2);
auto hashFunc = std::hash<std::string>();
auto name = i3==string::npos? filename : filename.substr(i3+1);
auto hashCode = hashFunc(filename);
replace_translate(name, "./\\ <>,;#@", "__________");
return format("%s_%s", name, hashCode);
}

