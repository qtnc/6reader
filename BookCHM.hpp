#ifndef _____BOOK_CHM_HPP_0
#define _____BOOK_CHM_HPP_0
#include "BookZip.hpp"

class BookCHM: public BookZip  {
public:
static std::shared_ptr<Book> open (const std::string& filename);
BookCHM () {}
bool load (const std::string& filename) override;

std::string getFirstPageURI () override;

std::shared_ptr<Resource> findResourceByURI (const std::string& uri) override;
int getFlags () override { return BOOK_HAS_TOC | BOOK_HAS_BOOKMARKS; }

~BookCHM () override {}
};

#endif
