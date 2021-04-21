#include "WXWidgets.hpp"
#include <wx/clipbrd.h>
#include "cpprintf.hpp"
#include<cstdio>
using namespace std;

void setClipboardText (const string& s) {
auto& cb = *wxClipboard::Get();
if (cb.Open()) {
finally __f([&](){ cb.Close(); });
cb.SetData(new wxTextDataObject(U(s)));
}}

string getClipboardText () {
auto& cb = *wxClipboard::Get();
if (cb.Open()) {
finally __f([&](){ cb.Close(); });
if (cb.IsSupported(wxDF_TEXT)) {
wxTextDataObject tdo;
cb.GetData(tdo);
return U(tdo.GetText());
}}
return "";
}

