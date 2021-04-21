#include "wxWidgets.hpp"
#include "BookHTML.hpp"
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
#include<memory>
using namespace std;

string makeAbsolutePath (string filename);

shared_ptr<Book> BookHTML::open (const string& filename) {
auto book = make_shared<BookHTML>();
if (book->load(filename)) return book;
else return nullptr;
}

bool BookHTML::load (const string& fn) {
filename = fn;
auto ff = new wxFFileInputStream(U(filename));
if (!ff || !ff->IsOk()) { println("Couldn't open %s", filename); return false; }
char cbuf[100] = {0};
if (!ff->ReadAll(cbuf, 99)) return false;
string sbuf = cbuf;
to_upper(sbuf);
if (sbuf.find("<HTML")==string::npos && sbuf.find("<!DOCTYPE HTML")==string::npos) {
println("%s isn't an HTML file", filename);
return false;
}
println("%s recognized as HTML file", filename);
ff->SeekI(0);
html = ReadAll(*ff);
return true;
}

std::shared_ptr<Resource> BookHTML::findResourceByURI (const std::string& uri) {
if (iequals(uri, "book.html")) {
auto resource = make_shared<Resource>();
resource->uri = uri;
return resource;
}
else return nullptr;
}

std::unique_ptr<wxInputStream> BookHTML::openResource (const std::shared_ptr<Resource>& resource) {
if (resource) return make_unique<wxMemoryInputStream>(html.data(), html.size());
else return nullptr;
}

