#include "BookUserData.hpp"
#include "stringUtils.hpp"
#include "wxWidgets.hpp"
#include<wx/xml/xml.h>
#include "XMLHelpers.hpp"
using namespace std;

bool BookUserData::empty () {
return false;
}

bool BookUserData::load (const std::string& filename) {
bookmarks.clear();
auto doc = make_unique<wxXmlDocument>(U(filename));
if (!doc || !doc->IsOk()) return false;
auto root = doc->GetRoot();
if (!root) return false;
auto bmRoot = FindNode(root, ByTag("bookmarks"));
if (bmRoot) for (auto item: FindNodes(bmRoot, ByTag("bookmark"))) {
auto name = U(item->GetAttribute("name"));
auto uri = U(item->GetAttribute("uri"));
auto position = U(item->GetAttribute("position"));
if (name.size() && uri.size() && position.size()) bookmarks.push_back(make_shared<Bookmark>(name, uri, position));
}
auto rp = FindNode(root, ByTag("readPosition"));
if (rp) {
lastUri = U(rp->GetAttribute("uri"));
lastPosition = U(rp->GetAttribute("position"));
}
return true;
}


bool BookUserData::save (const std::string& filename) {
auto doc = make_unique<wxXmlDocument>();
auto root = CreateElement("bookUserData");
doc->SetRoot(root);
if (lastUri.size()) {
auto rp = CreateElement("readPosition");
root->AddChild(rp);
rp->AddAttribute("uri", U(lastUri));
if (lastPosition.size()) rp->AddAttribute("position", U(lastPosition));
}
if (bookmarks.size()) {
auto bmRoot = CreateElement("bookmarks");
root->AddChild(bmRoot);
for (auto& bm: bookmarks) {
auto item = CreateElement("bookmark");
item->AddAttribute("name", U(bm->name));
item->AddAttribute("uri", U(bm->uri));
item->AddAttribute("position", U(bm->position));
bmRoot->AddChild(item);
}}
return doc->Save(U(filename));
}
