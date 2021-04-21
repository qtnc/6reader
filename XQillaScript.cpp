#include<regex>
#include "XQillaScript.hpp"
#include "XQScript.hpp"
#include "wxWidgets.hpp"
#include "App.hpp"
#include "UniversalSpeech.h"
#include "MainWindow.hpp"
#include "WebViewPopup.hpp"
#include <wx/process.h>
#include <wx/webview.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
#include<string>
#include<fstream>
#include<sstream>
#include<thread>
#include<utf8.h>
#include<typeinfo>
using namespace std;

template<class T, template<class, class...> class C, class... A> inline int elt (const T& value, const C<T, A...>& values) {
auto it = std::find(values.begin(), values.end(), value);
return it==values.end()? -1 : it-values.begin();
}

template <class T> inline int eltl (const T& value, const std::vector<T>& values) {
return elt(value, values);
}

bool XQillaScript::valid () {
return shortcutKey || name.size();
}

bool XQillaScript::load (const string& filename) {
ifstream in(filename);
if (!in) return false;
regex reg("^\\(:\\s*(\\w+)\\s*:\\s*(.*):\\)$");
smatch m;
string line;
while(getline(in, line)){
trim(line);
if (line.length()<=0) continue;
if (!regex_match(line, m, reg)) break;
string key = m[1], value = m[2];
trim(key); trim(value);
if (iequals(key, "name")) name = value;
else if (iequals(key, "description")) description = value;
else if (iequals(key, "icon")) icon  = value;
else if (iequals(key, "input")) input = eltl(value, {"none", "selection", "prompt"});
else if (iequals(key, "output")) output = eltl(value, {"page", "speak"});
else if (iequals(key, "shortcut")) assignShortcut(value);
else if (iequals(key, "flags")) {
auto flv = split(value, ",;", true);
for (auto& fl: flv) {
if (iequals(fl, "double-utf8")) flags |= XQS_FLAG_DOUBLE_UTF8;
}}}
file = replace_all_copy(filename, "\\", "/");
return true;
}

void XQillaScript::assignShortcut (const string& str) {
wxStringToKey(U(str), shortcutKey, shortcutModifiers);
}

string XQillaScript::shortcutToString () {
if (shortcutKey==0) return "";
return U(wxKeyToString(shortcutKey, shortcutModifiers, true));
}

void XQillaScript::execute (App& app, MainWindow* parent, wxWebView* webview) {
XQScript xqs;
string arg;
if (webview->HasSelection()) arg = U(webview->GetSelectedText());
if (input==XQS_INPUT_NONE) arg="";
else if (input==XQS_INPUT_SELECTION && arg.empty()) return;
else if (input==XQS_INPUT_PROMPT) {
arg = U(wxGetTextFromUser(U(translate("xqsPrompt")), U(name), U(arg), parent));
if (arg.empty()) return;
}
thread wkt([=,&app]()mutable{
trim(arg);
xqs.baseURI = file.substr(0, file.rfind('/')+1);
xqs.inputQuery = ReadAll(file);
if (arg.size()) xqs.variables["arg"] = arg;
bool success = executeXQillaScript(xqs);
RunEDT([=,&app](){ showResult(xqs.outputXML, xqs.outputError, success, app, parent, webview); });
}); 
wkt.detach();
}

void XQillaScript::showResult (const std::string& result, const std::string& error, bool success, struct App& app, struct MainWindow* parent, struct wxWebView* webview) {
if (success) {
string finalResult = result;
if ((flags&XQS_FLAG_DOUBLE_UTF8) && utf8::is_valid(finalResult.begin(), finalResult.end())) {
string tmp;
utf8::utf8to32(finalResult.begin(), finalResult.end(), back_inserter(tmp));
for (int antiloop = 0; antiloop<10000; antiloop++) {
auto it = utf8::find_invalid(tmp.begin(), tmp.end());
if (it==tmp.end()) break;
char cs[5] = {0};
uint32_t c = *reinterpret_cast<const unsigned char*>(&*it);
println("code point = %d, %c", c, (char)c);
utf8::utf32to8(&c, (&c)+1, cs);
tmp.insert((it-tmp.begin())+1, cs);
tmp.erase(it);
}
finalResult = tmp;
}
if (output==XQS_OUTPUT_PAGE) {
WebViewPopup wvp(parent);
wvp.SetPage(U(finalResult));
wvp.ShowModal();
}
else if (output==XQS_OUTPUT_SPEAK) {
speechSayU(result.c_str(), true);
}
//other cases
} else {
wxTextEntryDialog ted(parent, U(translate("xqsErrorM")), U(translate("xqsErrorT")), error, wxOK | wxTE_MULTILINE | wxTE_READONLY);
ted.ShowModal();
}}
