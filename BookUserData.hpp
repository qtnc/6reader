#ifndef _____BOOK_UDATA_HPP_0
#define _____BOOK_UDATA_HPP_0
#include<memory>
#include<vector>

struct Bookmark {
std::string name, uri, position;
Bookmark (const std::string& n, const std::string& u, const std::string& p): name(n), uri(u), position(p) {}
};

struct BookUserData {
std::shared_ptr<struct Book> book;
std::vector<std::shared_ptr<Bookmark>> bookmarks;
std::string lastUri, lastPosition;

BookUserData (std::shared_ptr<Book> b): book(b) {}
bool empty ();
bool load (const std::string& filename);
bool save (const std::string& filename);
};

#endif
