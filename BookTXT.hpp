#ifndef _____BOOK_TXT_HPP_0
#define _____BOOK_TXT_HPP_0
#include "Book.hpp"
#include<memory>

class BookTXT: public Book  {
std::string html;

public:
static std::shared_ptr<Book> open (const std::string& filename);
BookTXT () {}
bool load (const std::string& filename);
std::shared_ptr<Resource> findResourceByURI (const std::string& uri) override;
std::unique_ptr<wxInputStream> openResource (const std::shared_ptr<Resource>& resource) override;
std::string getFirstPageURI () override { return "book.html"; }

~BookTXT () override {}
};

#endif
