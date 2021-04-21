#include "wxWidgets.hpp"
#include "BookTXT.hpp"
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
#include<memory>
#include<sstream>
using namespace std;

string makeAbsolutePath (string filename);

shared_ptr<Book> BookTXT::open (const string& filename) {
auto book = make_shared<BookTXT>();
if (book->load(filename)) return book;
else return nullptr;
}

bool BookTXT::load (const string& fn) {
if (!iends_with(fn, ".txt")) {
println("%s isn't a TXT file", fn);
return false;
}
println("%s recognized as TXT file", fn);
filename = fn;
ostringstream out;
out << "<html><head><title>"
<<  filename.substr(1+min(filename.rfind('/'), filename.rfind('\\')))
<< "</title></head><body><xmp>";
ReadAll(filename, out);
html = out.str();
return true;
}

std::shared_ptr<Resource> BookTXT::findResourceByURI (const std::string& uri) {
if (iequals(uri, "book.html")) {
auto resource = make_shared<Resource>();
resource->uri = uri;
return resource;
}
else return nullptr;
}

std::unique_ptr<wxInputStream> BookTXT::openResource (const std::shared_ptr<Resource>& resource) {
if (resource) return make_unique<wxMemoryInputStream>(html.data(), html.size());
else return nullptr;
}

