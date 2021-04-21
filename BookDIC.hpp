#ifndef _____BOOK_DIC_HPP_0
#define _____BOOK_DIC_HPP_0
#include "Book.hpp"
#include<unordered_map>
#include<memory>

struct ResourceDIC: Resource {
size_t startPos, endPos;
};

class BookDIC: public Book  {
std::string text, lastText;
std::vector<std::shared_ptr<ResourceDIC>> index;

public:
static std::shared_ptr<Book> open (const std::string& filename);
BookDIC () {}
bool load (const std::string& filename);
bool loadIndex (const std::string& filename);
bool createIndex (const std::string& filename);
std::vector<std::shared_ptr<Resource>> findResourcesInIndex (const std::string& term) override;
std::shared_ptr<Resource> findResourceByURI (const std::string& uri) override;
std::unique_ptr<wxInputStream> openResource (const std::shared_ptr<Resource>& resource) override;
std::string getFirstPageURI () override;
std::string getNextPageURI (const std::string& currentURI) override;
std::string getPrevPageURI (const std::string& currentURI) override;

int getFlags () override { return BOOK_HAS_INDEX | BOOK_HAS_BOOKMARKS; }
~BookDIC () override {}
};

#endif
