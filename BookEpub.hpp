#ifndef _____BOOK_EPUB_HPP_0
#define _____BOOK_EPUB_HPP_0
#include "BookZip.hpp"
#include<unordered_map>

class BookEpub: public BookZip  {
std::unordered_map<std::string, std::shared_ptr<Resource>> items;
std::vector<std::shared_ptr<Resource>> spine;

public:
static std::shared_ptr<Book> open (const std::string& filename);
BookEpub () {}
bool load (const std::string& filename) override;
void loadTocNcx (const std::string& idNcx);
void loadTocNav (const std::string& idTocNav);

std::shared_ptr<Resource> findResourceById (const std::string& id);
std::shared_ptr<Resource> findResourceByURI (const std::string& uri) override;
std::vector<std::shared_ptr<Resource>> getSpine () override { return spine; }
std::vector<std::shared_ptr<Resource>> getResources () override;
int getFlags () override;

std::string getFirstPageURI () override;
std::string getNextPageURI (const std::string& currentURI) override;
std::string getPrevPageURI (const std::string& currentURI) override;

~BookEpub () override {}
};

#endif
