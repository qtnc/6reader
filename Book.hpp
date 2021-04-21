#ifndef _____BOOK_HPP_0
#define _____BOOK_HPP_0
#include<memory>
#include<vector>
#include<functional>
#include <wx/stream.h>

#define BOOK_HAS_TOC 1
#define BOOK_HAS_SPINE 2
#define BOOK_HAS_INDEX 4
#define BOOK_HAS_BOOKMARKS 8
#define BOOK_HAS_RESOURCES 16
#define BOOK_HAS_SEARCH 32

struct TocEntry {
std::string label, uri;
std::vector<std::shared_ptr<TocEntry>> children;
std::weak_ptr<TocEntry> parent;
std::shared_ptr<struct Resource> resource = nullptr;
struct wxTreeItemData* treeItem = nullptr;
};

struct Resource {
std::string id, name, uri, type;
bool nav=false, ncx=false, linear=true;
uintptr_t uidptr=0;
};

class Book {
protected:
std::string filename;

public:
static void init ();
static std::shared_ptr<Book> open (const std::string& filename);

virtual std::string getFileName () { return filename; }
virtual std::string getHashName () ;

virtual std::string getFirstPageURI () = 0;
virtual std::string getNextPageURI (const std::string& currentURI) { return ""; }
virtual std::string getPrevPageURI (const std::string& currentURI) { return ""; }

virtual std::shared_ptr<Resource> findResourceByURI (const std::string& uri) = 0;
virtual std::unique_ptr<wxInputStream> openResource (const std::shared_ptr<Resource>& resource) = 0;

virtual std::shared_ptr<TocEntry> getToc () { return nullptr; }
virtual std::vector<std::shared_ptr<Resource>> getSpine () { return {}; }
virtual std::vector<std::shared_ptr<Resource>> getResources () { return {}; }
virtual std::vector<std::shared_ptr<Resource>> findResourcesInIndex (const std::string& term) { return {}; }
virtual std::vector<std::shared_ptr<Resource>> findResourcesByFullSearch (const std::string& term) { return {}; }
virtual int getFlags () { return 0; }

virtual ~Book () { }

private:
static std::vector<std::function<std::shared_ptr<Book>(const std::string&)>> loaders;
static std::vector<std::pair<std::string,std::string>> filenameFilters;

public:
static const std::vector<std::pair<std::string,std::string>>& getOpenFileFilters () { init(); return filenameFilters; }
};

#endif
