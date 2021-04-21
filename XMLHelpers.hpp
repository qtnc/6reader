#ifndef ___XMLHELPERS___
#define ___XMLHELPERS___
#include <wx/xml/xml.h>
#include<vector>
#include<memory>
#include "cpprintf.hpp"

#define FN_RECURSE 1
#define FN_SINGLE 2
#define FN_SKIPROOT 4

template <class F>
std::vector<wxXmlNode*>& FindNodes (wxXmlNode* root, int flags, const F& func, std::vector<wxXmlNode*>& nodes) {
for (auto node = root->GetChildren(); node; node = node->GetNext()) {
if (func(node)) {
nodes.push_back(node);
if (flags&FN_SINGLE) goto end;
}
if (flags&FN_RECURSE) FindNodes(node, flags, func, nodes);
}
end: return nodes;
}

template <class F>
std::vector<wxXmlNode*> FindNodes (wxXmlNode* root, int flags, const F& func) {
std::vector<wxXmlNode*> nodes;
if ((!(flags&FN_SKIPROOT)) && func(root)) {
nodes.push_back(root);
if (flags&FN_SINGLE) goto end;
}
FindNodes(root, flags, func, nodes);
end: return nodes;
}

template <class F>
std::vector<wxXmlNode*> FindNodes (wxXmlNode* root, const F& func) {
return FindNodes(root, 0, func);
}

template <class F> 
wxXmlNode* FindNode (wxXmlNode* root, int flags, const F& func) {
auto re = FindNodes(root, flags | FN_SINGLE, func);
return re.size()? re[0] : nullptr;
}

template <class F> 
wxXmlNode* FindNode (wxXmlNode* root, const F& func) {
return FindNode(root, FN_SINGLE, func);
}

struct NodeFinder {
virtual bool operator() (const wxXmlNode* node) const = 0;
virtual ~NodeFinder () = default;
};

struct ByTagNodeFinder: NodeFinder {
wxString name;
ByTagNodeFinder (const wxString& n): name(n) {}
virtual bool operator() (const wxXmlNode* node) const override {   return node && node->GetType()==wxXML_ELEMENT_NODE && node->GetName()==name;   }
};

struct ByAttrNodeFinder: NodeFinder {
wxString name;
ByAttrNodeFinder (const wxString& n): name(n) {}
virtual bool operator() (const wxXmlNode* node) const override { return node && node->GetType()==wxXML_ELEMENT_NODE && node->HasAttribute(name); }
};

struct ByAttrValNodeFinder: NodeFinder {
wxString name, value;
ByAttrValNodeFinder (const wxString& n, const wxString& v): name(n), value(v)  {}
virtual bool operator() (const wxXmlNode* node) const override { return node && node->GetType()==wxXML_ELEMENT_NODE && node->HasAttribute(name) && node->GetAttribute(name)==value; }
};

struct AndNodeFinder: NodeFinder {
const NodeFinder &nf1, &nf2;
AndNodeFinder (const NodeFinder& a, const NodeFinder& b): nf1(a), nf2(b) {}
virtual bool operator() (const wxXmlNode* node) const override { return nf1(node) && nf2(node); }
};

struct OrNodeFinder: NodeFinder {
const NodeFinder &nf1, &nf2;
OrNodeFinder (const NodeFinder& a, const NodeFinder& b): nf1(a), nf2(b) {}
virtual bool operator() (const wxXmlNode* node) const override { return nf1(node) || nf2(node); }
};

#define ByTag(NAME) *std::make_unique<ByTagNodeFinder>(NAME)
#define ByAttr(NAME) *std::make_unique<ByAttrNodeFinder>(NAME)
#define ByAttrVal(NAME,VALUE) *std::make_unique<ByAttrValNodeFinder>(NAME, VALUE)
#define And(A,B) *std::make_unique<AndNodeFinder>(A, B)
#define Or(A,B) *std::make_unique<OrNodeFinder>(A, B)

#define CreateElement(NAME) new wxXmlNode(wxXML_ELEMENT_NODE, NAME)
#define CreateTextNode(TXT) new wxXmlNode(wxXML_ELEMENT_TEXT, wxEmptyString, TXT)

std::unique_ptr<wxXmlDocument> LoadHTML (const std::string& html);



#endif
