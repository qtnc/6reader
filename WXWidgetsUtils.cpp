#include "wxWidgets.hpp"
#include <wx/accel.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include<sstream>
#include<memory>
using namespace std;

bool wxStringToKey (const wxString& str, int& key, int& modifiers) {
wxAcceleratorEntry e;
if (!e.FromString(str)) return false;
int flags = e.GetFlags();
key = e.GetKeyCode();
modifiers=0;
if (flags&wxACCEL_SHIFT) modifiers|=wxMOD_SHIFT;
if (flags&wxACCEL_CTRL) modifiers|=wxMOD_CONTROL;
if (flags&wxACCEL_ALT) modifiers|=wxMOD_ALT;
return true;
}

wxString wxKeyToString (int key, int modifiers, bool i18n) {
int flags=0;
if (modifiers&wxMOD_SHIFT) flags|=wxACCEL_SHIFT;
if (modifiers&wxMOD_CONTROL) flags|=wxACCEL_CTRL;
if (modifiers&wxMOD_ALT) flags|=wxACCEL_ALT;
wxAcceleratorEntry e(flags, key);
if (i18n) return e.ToString();
else return e.ToRawString();
}

string ReadAll (wxInputStream& in) {
ostringstream out;
ReadAll(in, out);
return out.str();
}

void ReadAll (wxInputStream& in, ostream& out) {
if (!in.IsOk()) return;
auto buf = make_unique<char[]>(4096);
while(true){
in.Read(&buf[0], 4096);
auto n = in.LastRead();
if (n<=0) break;
out.write(&buf[0], n);
}
}

string ReadAll (const std::string& fn) {
auto ff = make_unique<wxFFileInputStream>(U(fn));
if (!ff || !ff->IsOk()) return "";
return ReadAll(*ff);
}

void ReadAll (const std::string& fn, ostream& out) {
auto ff = make_unique<wxFFileInputStream>(U(fn));
if (!ff || !ff->IsOk()) return;
ReadAll(*ff, out);
}
