#include "wxWidgets.hpp"
#include "BookDIC.hpp"
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
#include<memory>
#include<sstream>
#include<fstream>
using namespace std;

string makeAbsolutePath (string filename);

string to_lower_locale (const string& s) {
auto u = U(s);
u.MakeLower();
return U(u);
}

bool resourceLess (const shared_ptr<Resource>& r1, const shared_ptr<Resource>& r2) {
return r1->name < r2->name;
}

vector<shared_ptr<ResourceDIC>>::iterator findEntry (vector<shared_ptr<ResourceDIC>>& index, const string& uri, bool exact=true) {
auto rtmp = make_shared<ResourceDIC>();
rtmp->name = rtmp->uri = to_lower_locale(uri);
auto it = lower_bound(index.begin(), index.end(), rtmp, resourceLess);
if (exact && it!=index.end() && (*it)->uri!=uri) return index.end();
else return it;
}

shared_ptr<ResourceDIC> addEntry (vector<shared_ptr<ResourceDIC>>& index, const string& name, size_t startPos, size_t endPos) {
auto entry = make_shared<ResourceDIC>();
entry->name = entry->uri = name;
entry->startPos = startPos;
entry->endPos = endPos;
entry->uidptr = reinterpret_cast<uintptr_t>(&entry->uri);
index.push_back(entry);
return entry;
}

shared_ptr<Book> BookDIC::open (const string& filename) {
auto book = make_shared<BookDIC>();
if (book->load(filename)) return book;
else return nullptr;
}

bool BookDIC::load (const string& fn) {
if (!iends_with(fn, ".dic")) {
println("%s isn't a DIC file", filename);
println("%s recognized as DIC file", filename);
return false;
}
filename = fn;
string idxfn = filename.substr(0, filename.size() -4) + ".idx";
text = ReadAll(filename);
return loadIndex(idxfn) || createIndex(idxfn);
}

std::shared_ptr<Resource> BookDIC::findResourceByURI (const std::string& uri) {
auto it = findEntry(index, uri, true);
return it!=index.end()? *it : nullptr;
}

std::unique_ptr<wxInputStream> BookDIC::openResource (const std::shared_ptr<Resource>& resource) {
if (!resource) return nullptr;
auto it = static_pointer_cast<ResourceDIC>(resource);
auto article = text.substr(it->startPos, it->endPos-it->startPos);
trim(article);
ostringstream out;
 out << "<!DOCTYPE HTML>" << endl;
out << "<html><head><meta charset=\"utf-8\" />" << endl;
out << "<title>" << it->name << "</title>" << endl;
out << "</head><body><pre>" << endl;
out << article;
out << "</pre></body></html>" << endl;
lastText = out.str();
return make_unique<wxMemoryInputStream>(lastText.data(), lastText.size());
}

std::string BookDIC::getFirstPageURI () {
return index[0]->uri;
}

std::string BookDIC::getNextPageURI (const std::string& currentURI) {
auto it = findEntry(index, currentURI);
if (it==index.end()) return "";
++it;
if (it==index.end()) return "";
return (*it)->uri;
}

std::string BookDIC::getPrevPageURI (const std::string& currentURI) {
auto it = findEntry(index, currentURI);
if (it==index.end() || it==index.begin()) return "";
--it;
return (*it)->uri;
}

std::vector<std::shared_ptr<Resource>> BookDIC::findResourcesInIndex (const std::string& term0) {
if (term0.empty()) return {};
string term = to_lower_locale(term0);
auto star = term.find('*'), joker = term.find('?');
if (star==string::npos && joker==string::npos) {
auto start = findEntry(index, term, false);
auto finish = find_if(start, index.end(), [&](auto& r){ return !starts_with(r->name, term); });
return vector<shared_ptr<Resource>>(start, finish);
} 
else {
string partialTerm = term.substr(0, std::min(star, joker));
bool partialTermNonEmpty = partialTerm.empty();
auto start = partialTermNonEmpty? findEntry(index, partialTerm, false) : index.begin();
auto finish = partialTermNonEmpty? find_if(start, index.end(), [&](auto& r){ return !starts_with(r->name, partialTerm); }) : index.end();
auto uTerm = U(term);
vector<shared_ptr<Resource>> results;
copy_if(start, finish, back_inserter(results), [&](auto& r){ return U(r->name).Matches(uTerm); });
return results;
}}

template<class T> inline bool readval (istream& in, T& val) {
return !!in.read(reinterpret_cast<char*>(&val), sizeof(val));
}

template<class T> inline bool writeval (ostream& out, const T& val) {
return !!out.write(reinterpret_cast<const char*>(&val), sizeof(val));
}

bool BookDIC::loadIndex (const string& filename) {
println("Loading DIC index in %s...", filename);
ifstream in; 
in.open(filename, ios::binary);
if (!in) {
println("Couldn't open DIC index in %s", filename);
return false;
}
index.clear();
size_t startPos, endPos, nameLen;
string name('\0', 64);
while(in && readval(in, nameLen) && readval(in, startPos) && readval(in, endPos)) {
if (nameLen==0) break;
name.resize(nameLen);
in.read(const_cast<char*>(name.data()), nameLen);
addEntry(index, name, startPos, endPos);
}
return true;
}

bool BookDIC::createIndex (const string& filename) {
println("Creating DIC index in %s...", filename);
if (text.empty()) return false;
index.clear();

unordered_map<string,shared_ptr<ResourceDIC>> entries;
istringstream in(text, ios::binary);
string line;
size_t pos = 0;
bool wasBlank = true;
shared_ptr<ResourceDIC> entry = nullptr;
while(true){
pos = in.tellg();
if (!getline(in, line)) break;
trim(line);
if (line.empty() == wasBlank) continue;
else if (line.empty()) {
wasBlank=true;
if (entry) entry->endPos = in.tellg();
}
else if (wasBlank) {
wasBlank=false;
string name = to_lower_locale(line);
entry = entries[name];
if (!entry) {
entry = addEntry(index, name, pos, pos);
entries[name] = entry;
}}}
if (entry) entry->endPos = text.size();
stable_sort(index.begin(), index.end(), resourceLess);
println("Created DIC index with %d entries", index.size());

ofstream out(filename, ios::binary);
for (auto& r: index) {
writeval(out, r->name.size());
writeval(out, r->startPos);
writeval(out, r->endPos);
out.write(r->name.data(), r->name.size());
}
for (int i=0; i<12; i++) writeval(out, 0);
println("Created and saved DIC index in %s", filename);
return true;
}
