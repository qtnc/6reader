#include "BookZip.hpp"
#include "wxWidgets.hpp"
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/dir.h>
#include<memory>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
using namespace std;

#undef LoadString

unique_ptr<wxInputStream> BookZip::openFile (const std::string& filenameInArchive) {
if (isFolder) {
println("Opening %s in %s", filenameInArchive, filename);
auto ff = make_unique<wxFFileInputStream>(U(filename + filenameInArchive));
if (ff->IsOk()) return std::move(ff);
else return nullptr;
}
else {
auto ff = new wxFFileInputStream(U(filename));
if (!ff || !ff->IsOk()) return nullptr;
auto zf = make_unique<wxZipInputStream>(ff);
if (!zf || !zf->IsOk()) return nullptr;
wxZipEntry* entry = nullptr;
while( (entry = zf->GetNextEntry()) ) {
auto name = U(entry->GetName(wxPATH_UNIX));
if (name == filenameInArchive) return move(zf);
}}
return nullptr;
}

unique_ptr<wxInputStream> BookZip::openFile (const function<bool(const string&)>& func) {
if (isFolder) {
wxDir dir(U(filename));
wxString file, result;
if (dir.GetFirst(&file)) do {
if (func(U(file))) { result=file; break; }
} while (dir.GetNext(&file));
if (result.empty()) return nullptr;
auto ff = make_unique<wxFFileInputStream>(dir.GetNameWithSep() + result);
if (ff->IsOk()) return std::move(ff);
else return nullptr;
}
else {
auto ff = new wxFFileInputStream(U(filename));
if (!ff || !ff->IsOk()) return nullptr;
auto zf = make_unique<wxZipInputStream>(ff);
if (!zf || !zf->IsOk()) return nullptr;
wxZipEntry* entry = nullptr;
while( (entry = zf->GetNextEntry()) ) {
auto name = U(entry->GetName(wxPATH_UNIX));
if (func(name)) return move(zf);
}}
return nullptr;
}

unique_ptr<wxInputStream> BookZip::openResource (const std::shared_ptr<Resource>& resource) {
return openFile(resource->uri);
}

string makeRelativePath (const string& src, const string& dst) {
auto dsts = split(dst, "/\\", true), srcs = split(src, "/\\", true);
if (dsts.size() && !ends_with(dst, "/")) dsts.pop_back();
while(srcs.size() && dsts.size() && srcs[0]=="..") { srcs.erase(srcs.begin()); dsts.pop_back(); }
do {
auto it = find(srcs.begin(), srcs.end(), "..");
if (it==srcs.end()) break;
auto pos = it - srcs.begin();
srcs.erase(it);
srcs.erase(srcs.begin() + pos -1);
}while(true);
dsts.insert(dsts.end(), srcs.begin(), srcs.end());
return join(dsts, "/");
}

string makeAbsolutePath (string filename) {
if (!ends_with(filename, "/") && !ends_with(filename, "\\") && wxDirExists(U(filename))) filename+="/";
wxFileName wxfn(U(filename));
wxfn.MakeAbsolute();
return U(wxfn.GetFullPath());
}


bool BookZip::load (const string& fn) {
filename = makeAbsolutePath(fn);
println("Loading book %s...", filename);
isFolder = wxDirExists(U(filename));
if (!isFolder) {
auto ff = new wxFFileInputStream(U(filename));
if (!ff || !ff->IsOk()) { println("Couldn't open %s", filename); return false; }
auto zf = make_unique<wxZipInputStream>(ff);
if (!zf || !zf->IsOk()) { println("Not a zip file: %s", filename); return false; }
}
return true;
}

