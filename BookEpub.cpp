#include "BookEpub.hpp"
#include "wxWidgets.hpp"
#include <wx/xml/xml.h>
#include "stringUtils.hpp"
#include "XMLHelpers.hpp"
#include "cpprintf.hpp"
using namespace std;

#undef LoadString

string removeHashPart (const string& s) {
auto i = s.find('#');
if (i!=string::npos) return s.substr(0, i);
else return s;
}

shared_ptr<Book> BookEpub::open (const string& filename) {
auto book = make_shared<BookEpub>();
if (book->load(filename)) return book;
else return nullptr;
}

bool BookEpub::load (const string& fn) {
if (!BookZip::load(fn)) return false;
string containerPath = "META-INF/container.xml";
auto zf = openFile(containerPath);
if (!zf) { println("Couldn't find %s in %s", containerPath, filename); return false; }
auto doc = make_unique<wxXmlDocument>(*zf);
if (!doc) { println("%s in %s isn't a valid XML document", containerPath, filename); return false; }
auto container = doc->GetRoot();
if (!container) { println("%s in %s isn't a valid XML document", containerPath, filename); return false; }
auto rootfile = FindNode(container, FN_RECURSE, ByTag("rootfile"));
if (!rootfile) { println("Couldn't find element rootfile in %s", containerPath); return false; }
auto opfPath = UFN(rootfile->GetAttribute("full-path"));
if (opfPath.empty()) { println("Couldn't find attribute full-path in element rootfile in %s", containerPath); return false; }
zf = openFile(opfPath);
if (!zf) { println("Couldn't find OPF package file %s in %s", opfPath, filename); return false; }
doc = make_unique<wxXmlDocument>(*zf);
if (!doc) { println("%s in %s isn't a valid XML document", opfPath, filename); return false; }
auto opf = doc->GetRoot();
if (!opf) { println("%s in %s isn't a valid XML document", opfPath, filename); return false; }
auto manifest = FindNode(opf, ByTag("manifest"));
if (!manifest) { println("Couldn't find manifest element in %s", opfPath); return false; }
println("%s recognized as epub book", filename);

string idTocNav, idTocNcx;
if (manifest) for (auto it: FindNodes(manifest, ByTag("item")))  {
string id = U(it->GetAttribute("id")),  name = U(it->GetAttribute("name")),  href = urldecode(U(it->GetAttribute("href"))),  type = U(it->GetAttribute("media-type")),  propstr = U(it->GetAttribute("properties"));
if (id.empty() || href.empty()) continue;
auto props = split(propstr, ",; ", true);
href = makeRelativePath(href, opfPath);
auto item = make_shared<Resource>();
item->uri = href;
item->id = id;
item->name = name;
item->type = type;
item->uidptr = reinterpret_cast<uintptr_t>(&item->uri);
if (find(props.begin(), props.end(), "nav")!=props.end()) idTocNav = id; 
items[item->id] = item;
}

auto spine = FindNode(opf, ByTag("spine"));
if (spine) {
idTocNcx = U(spine->GetAttribute("toc")); 
auto lastLinear = true;
for (auto itref: FindNodes(spine, ByTag("itemref"))) {
string id = U(itref->GetAttribute("idref"));
auto linear = itref->GetAttribute("linear", lastLinear?"yes":"no")=="yes";
lastLinear=linear;
if (id.empty()) continue;
auto item = findResourceById(id);
if (!item) continue;
item->linear = linear;
this->spine.push_back(item);
}
}//if spine

if (idTocNcx.size()) loadTocNcx(idTocNcx);
if (idTocNav.size()) loadTocNav(idTocNav);
return true;
}

shared_ptr<Resource> BookEpub::findResourceById (const std::string& id) {
auto it = items.find(id);
return it==items.end()? nullptr : it->second;
}

shared_ptr<Resource> BookEpub::findResourceByURI (const std::string& uri) {
auto it = find_if(items.begin(), items.end(), [&](auto& p){
return p.second->uri == uri;
});
return it==items.end()? nullptr : it->second;
}

std::string BookEpub::getFirstPageURI () {
shared_ptr<Resource> it;
for (auto item: spine) if (item->linear) { it=item; break; }
if (!it && spine.size()) it = spine[0];
if (!it && toc && toc->children.size()) it = findResourceByURI(removeHashPart(toc->children[0]->uri));
return it? it->uri : "";
}

std::string BookEpub::getNextPageURI (const std::string& uri) {
auto item = findResourceByURI(uri);
if (!item) return "";
auto it = find(spine.begin(), spine.end(), item);
if (it==spine.end()) return "";
while(++it!=spine.end() && !(*it)->linear);
if (it==spine.end()) return "";
return (*it)->uri;
}

std::string BookEpub::getPrevPageURI (const std::string& uri) {
auto item = findResourceByURI(uri);
if (!item) return "";
auto it = find(spine.begin(), spine.end(), item);
if (it==spine.end() || it==spine.begin()) return "";
while(--it!=spine.end() && it!=spine.begin() && !(*it)->linear);
if (it==spine.end() || it==spine.begin()) return "";
return (*it)->uri;
}

vector<shared_ptr<Resource>> BookEpub::getResources () {
vector<shared_ptr<Resource>> resources;
for (auto& p: items) resources.push_back(p.second);
return resources;
}

int BookEpub::getFlags () {
int flags = BOOK_HAS_RESOURCES | BOOK_HAS_BOOKMARKS;
if (toc && !toc->children.empty()) flags |= BOOK_HAS_TOC;
if (!spine.empty()) flags |= BOOK_HAS_SPINE;
return flags;
}

void loadNavPoints (BookEpub& book, shared_ptr<TocEntry> toc, wxXmlNode* root, const string& ncxuri) {
for (auto navPoint: FindNodes(root, FN_SKIPROOT, ByTag("navPoint"))) {
auto navLabel = FindNode(navPoint, ByTag("navLabel"));
if (!navLabel) continue;
auto labelText = FindNode(navLabel, ByTag("text"));
if (!labelText) continue;
auto label = U(labelText->GetNodeContent());
if (label.empty()) continue;
auto content = FindNode(navPoint, ByTag("content"));
string uri;
if (content) uri = U(content->GetAttribute("src"));
if (uri.size()) uri = makeRelativePath(uri, ncxuri);
auto item = make_shared<TocEntry>();
item->label = label;
item->uri = uri;
item->parent = toc;
item->resource = book.findResourceByURI(removeHashPart(uri));
if (item->resource && item->resource->name.empty() && label.size()) item->resource->name = label;
toc->children.push_back(item);
loadNavPoints(book, item, navPoint, ncxuri);
}}

void BookEpub::loadTocNcx (const string& idNcx) {
auto resource = findResourceById(idNcx);
if (!resource) return;
auto zf = openFile(resource->uri);
if (!zf) return;
auto doc = make_unique<wxXmlDocument>(*zf);
if (!doc) return;
auto root = doc->GetRoot();
if (!root) return;
auto navMap = FindNode(root, FN_RECURSE, ByTag("navMap"));
if (!navMap) return;
resource->ncx = true;
toc = make_shared<TocEntry>();
loadNavPoints(*this, toc, navMap, resource->uri);
}

void loadTocItems (BookEpub& book, shared_ptr<TocEntry> toc, wxXmlNode* root, const string& tocuri) {
for (auto li: FindNodes(root, ByTag("li"))) {
auto a = FindNode(li, Or(ByTag("a"), ByTag("span")));
auto sublist = FindNode(li, Or(ByTag("ol"), ByTag("ul")));
if (!a) continue;
auto item = make_shared<TocEntry>();
auto label = U(a->GetNodeContent());
auto href = U(a->GetAttribute("href"));
if (href.size() && href!="#") href = makeRelativePath(href, tocuri);
item->label = label;
item->uri = href;
item->parent = toc;
item->resource = book.findResourceByURI(removeHashPart(href));
if (item->resource && item->resource->name.empty() && label.size()) item->resource->name = label;
if (sublist) loadTocItems(book, item, sublist, tocuri);
toc->children.push_back(item);
}}

void BookEpub::loadTocNav (const string& idTocNav) {
auto resource = findResourceById(idTocNav);
if (!resource) return;
auto zf = openFile(resource->uri);
if (!zf) return;
auto doc = make_unique<wxXmlDocument>(*zf);
if (!doc) return;
auto root = doc->GetRoot();
if (!root) return;
root = FindNode(root, FN_RECURSE, ByAttrVal("epub:type", "toc"));
if (!root) return;
root = FindNode(root, FN_RECURSE, Or(ByTag("ol"), ByTag("ul")));
if (!root) return;
resource->nav = true;
toc = make_shared<TocEntry>();
loadTocItems(*this, toc, root, resource->uri);
}
