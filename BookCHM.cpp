#include "BookCHM.hpp"
#include "wxWidgets.hpp"
#include <wx/xml/xml.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include "stringUtils.hpp"
#include "XMLHelpers.hpp"
#include "cpprintf.hpp"
#include "App.hpp"
using namespace std;

std::shared_ptr<Resource> BookCHM::findResourceByURI (const std::string& uri) {
auto resource = make_shared<Resource>();
resource->uri = uri;
return resource;
}

bool tryDecompileHH (string& filename, const string& hashname) {
filename = makeAbsolutePath(filename);
auto ff = make_unique<wxFFileInputStream>(U(filename));
if (!ff || !ff->IsOk()) { println("Couldn't open %s", filename); return false; }
char buf[4] = {0};
ff->Read(buf,4);
if (ff->LastRead()!=4) { println("Couldn't read %s", filename); return false; }
if (strncmp(buf, "ITSF", 4)) { println("%s isn't a CHM ITSF file", filename); return false; }
println("%s recognized as CHM ITSF file", filename);
ff.reset();
auto tmpdir = U(wxGetApp() .userDir); //U(wxFileName::GetTempDir());
if (ends_with(tmpdir, "/") || ends_with(tmpdir, "\\")) tmpdir = tmpdir.substr(0, tmpdir.size() -1);
auto hhdir = tmpdir + "\\" + hashname;
if (wxDirExists(U(hhdir))) { filename=hhdir; return true; }
println("Decompiling CHM ITSF file %s into directory %s...", filename, hhdir);
auto cmd = format("hh -decompile %s %s", hhdir, filename);
auto result =  wxExecute(U(cmd), wxEXEC_SYNC | wxEXEC_NOEVENTS | wxEXEC_HIDE_CONSOLE);
println("Command %s returned %d", cmd, result);
if (result) return false;
filename=hhdir;
return wxDirExists(U(hhdir));
}

void loadToc (shared_ptr<TocEntry> toc, wxXmlNode* root) {
for (auto li: FindNodes(root, ByTag("li"))) {
auto node = FindNode(li, ByTag("object"));
if (!node) continue;
auto paramName = FindNode(node, And(ByTag("param"), ByAttrVal("name", "Name")));
if (!paramName) continue;
auto label = U(paramName->GetAttribute("value"));
if (label.empty()) continue;
auto paramLocal = FindNode(node, And(ByTag("param"), ByAttrVal("name", "Local")));
auto uri = paramLocal? U(paramLocal->GetAttribute("value")) :"";
auto entry = make_shared<TocEntry>();
entry->label = label;
entry->uri = uri;
toc->children.push_back(entry);
node = FindNode(li, ByTag("ul"));
if (node) loadToc(entry, node);
}}

shared_ptr<Book> BookCHM::open (const string& filename) {
auto book = make_shared<BookCHM>();
if (book->load(filename)) return book;
else return nullptr;
}

bool BookCHM::load (const string& fn) {
filename=fn;
if (!tryDecompileHH(filename, getHashName()) || !BookZip::load(filename)) return false;
auto zf = openFile([&](const string& name){ return iends_with(name, ".hhc"); });
if (!zf) { println("Couldn't find HTML help content file (HHC) in %s", filename); return false; }
auto hhc  = ReadAll(*zf);
auto doc = LoadHTML(hhc);
if (!doc) { println("HTML help content file (HHC) parse error in %s", filename); return false; }
auto root = doc->GetRoot();
if (!root) return false;
root = FindNode(root, ByTag("body"));
if (!root) return false;
root = FindNode(root, ByTag("ul"));
if (!root) return false;
toc = make_shared<TocEntry>();
loadToc(toc, root);
return true;
}

string findFirstNonEmptyURI (shared_ptr<TocEntry> toc) {
string re;
if (toc) for (auto entry: toc->children) {
if (entry->uri.size()) re = entry->uri;
else re = findFirstNonEmptyURI(entry);
if (re.size()) return re;
}
return re;
}

std::string BookCHM::getFirstPageURI () {
return findFirstNonEmptyURI(toc);
}

