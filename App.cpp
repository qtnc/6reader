#include "App.hpp"
#include "WebViewBookHandler.hpp"
#include "MainWindow.hpp"
#include "XQillaScript.hpp"
#include "UniversalSpeech.h"
#include "WXWidgets.hpp"
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include "stringUtils.hpp"
#include "cpprintf.hpp"
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/snglinst.h>
#include <wx/ipc.h>
#include <wx/datetime.h>
#include <wx/translation.h>
#include "wx/webview.h"
#include "wx/msw/webview_ie.h"
#include "wx/filesys.h"
#include "wx/fs_arc.h"
#include "wx/fs_mem.h"
#include<string>
#include<vector>
#include<unordered_map>
using namespace std;

wxIMPLEMENT_APP(App);

struct CustomFileTranslationLoader: wxTranslationsLoader {
virtual wxMsgCatalog* LoadCatalog (const wxString& domain, const wxString& lang) final override {
string filename = "lang/" + U(domain) + "_" + U(lang) + ".mo";
println("Loading WXWidgets translations (domain=%s, lang=%s) in %s...", domain, lang, filename);
wxMsgCatalog* re = nullptr;
bool existing = false;
{ ifstream in(filename, ios::binary); if (in) existing=true; }
if (existing) re = wxMsgCatalog::CreateFromFile( U(filename), domain );
println(re? "Loaded WXWidgets translations (domain=%s, lang=%s) in %s" : "Couldn't load WXWidgets translations (domain=%s, lang=%s) in %s: not found", domain, lang, filename);
return re;
}
     virtual wxArrayString GetAvailableTranslations(const wxString& domain) const final override {
vector<string> langs = { "fr", "en", "it", "es", "pt", "de", "ru" };
wxArrayString v;
for (auto& s: langs) v.push_back(s);
return v;
}};

bool App::OnInit () {
initDirs();
initConfig();
initLocale();
initTranslations();
initWebview();
initScriptList();
initMisc();

if (!wxApp::OnInit()) return false;

win = new MainWindow(*this);
win->Show(true);
return true;
}

void App::OnInitCmdLine (wxCmdLineParser& cmd) {
wxApp::OnInitCmdLine(cmd);
cmd.AddParam(wxEmptyString, wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL);
}

bool App::OnCmdLineParsed (wxCmdLineParser& cmd) {
int n=cmd.GetParamCount(); 
if (n>=1) {
fileToOpen = U(cmd.GetParam(0));
}
else {
fileToOpen = config.get("book.last.filename", fileToOpen);
uriToOpen = config.get("book.last.uri", uriToOpen);
}
if (n>=2) uriToOpen = U(cmd.GetParam(1));
return true;
}


bool App::initDirs () {
cout << "Retrieving standard directories..." << endl;
SetAppName(APP_NAME);
SetClassName(APP_NAME);
SetVendorName(APP_VENDOR);

auto& stdPaths = wxStandardPaths::Get();
appDir = wxFileName(stdPaths.GetExecutablePath()).GetPath();
userDir = stdPaths.GetUserDataDir();
userLocalDir = stdPaths.GetUserLocalDataDir();

cout << "userDir = " << userDir << endl;
cout << "userLocalDir = " << userLocalDir << endl;
cout << "appDir = " << appDir << endl;

auto userDirFn = wxFileName::DirName(userDir);
auto userLocalDirFn = wxFileName::DirName(userLocalDir);

pathList.Add(userDir);
//pathList.Add(userLocalDir);
pathList.Add(appDir);

return 
userDirFn .Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)
&& userLocalDirFn .Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)
&& userDirFn.IsDirReadable()
&& userLocalDirFn.IsDirReadable();
}

bool App::initConfig () {
cout << "Loading user config..." << endl;
string configIniPath = UFN(pathList.FindAbsoluteValidPath(CONFIG_FILENAME));
if (configIniPath.empty()) cout << "No " << CONFIG_FILENAME << " found, fallback to defaults" << endl;
else {
cout << CONFIG_FILENAME << " found: " << configIniPath << endl;
config.setFlags(PM_BKESC);
config.load(configIniPath);
}
//todo: read config from map
return true;
}

string App::findWritablePath (const string& wantedPath) {
int lastSlash = wantedPath.find_last_of("/\\");
string path, file;
if (lastSlash==string::npos) { path = ""; file=wantedPath; }
else { path=wantedPath.substr(0, lastSlash); file=wantedPath.substr(lastSlash+1); }
for (int i=pathList.GetCount() -1; i>=0; i--) {
auto wxfn = wxFileName(pathList.Item(i), wxEmptyString);
if (wxfn.IsFileWritable() || wxfn.IsDirWritable()) {
string dirpath = UFN(wxfn.GetFullPath());
if (path.size()) {
if (!ends_with(dirpath, "/") && !ends_with(dirpath, "\\")) dirpath += "/";
dirpath += path;
}
if (wxFileName(dirpath, "").Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL)) {
wxfn = wxFileName(dirpath, file);
if (wxfn.IsFileWritable() || wxfn.IsDirWritable()) {
return UFN(wxfn.GetFullPath());
}}}}
return string();
}

bool App::saveConfig () {
string filename = findWritablePath(CONFIG_FILENAME);
if (filename.empty()) {
cout << "No valid writable path found to save configuration " << CONFIG_FILENAME << endl;
return false;
}
cout << "Saving configuration to " << filename << endl;
//todo: save config to map
return config.save(filename);
}

bool App::initLocale () {
cout << "Initializing locale..." << endl;
locale = config.get("locale", "default");
wxlocale = new wxLocale();
if (locale=="default") {
cout << "No locale set in the configuration, retrieving system default" << endl;
wxlocale->Init();
}
else {
auto info = wxLocale::FindLanguageInfo(U(locale));
if (info) wxlocale->Init(info->Language);
else cout << "Couldn't find locale information for " << locale << endl;
}
this->locale = U(wxlocale->GetCanonicalName());
auto& translations = *wxTranslations::Get();
translations.SetLoader(new CustomFileTranslationLoader());
translations.AddStdCatalog();
cout << "Locale configured to " << locale << endl;
return true;
}

bool App::initTranslations () {
cout << "Loading translations..." << endl;
vector<string> locales = {
config.get("locale", locale),
config.get("locale", locale).substr(0, 5),
config.get("locale", locale).substr(0, 2),
locale,
locale.substr(0, 5),
locale.substr(0, 2),
"en"
};
for (string& l: locales) {
string transPath = UFN(pathList.FindAbsoluteValidPath(format("lang/app_%s.properties", l)));
if (!transPath.empty()) {
cout << "Translations found for locale " << l << " in " << transPath << endl;
lang.setFlags(PM_BKESC);
lang.load(transPath);
break;
}}
return true;
}

bool App::initWebview () {
wxWebViewIE::MSWSetEmulationLevel(wxWEBVIEWIE_EMU_IE11);
bookFSHandler = new WebViewBookHandler();
bookFsHandlerSp = bookFSHandler;
return true;
}

wxWebView* App::createWebView (wxWindow* parent, int id, const wxString& initialPage) {
auto webview = wxWebView::New(parent, id, initialPage);
webview->EnableContextMenu(false);
webview->RegisterHandler(bookFsHandlerSp);
return webview;
}

bool App::initMisc () {
wxInitAllImageHandlers();
auto ff = make_unique<wxFFileInputStream>(appDir + "/loadscript.js");
onLoadScript = U(ReadAll(*ff));
return true;
}

bool App::initScriptList () {
for (int i=pathList.GetCount() -1; i>=0; i--) {
auto dir = wxDir(pathList.Item(i) + "/scripts");
if (!dir.IsOpened()) continue;
wxString wxfn;
if (dir.GetFirst(&wxfn, "*.xq")) do {
wxfn = dir.GetNameWithSep() + wxfn;
auto xq = make_shared<XQillaScript>();
if (xq->load(U(wxfn)) && xq->valid()) xqScripts.push_back(xq);
} while(dir.GetNext(&wxfn));
}
return true;
}


void App::changeLocale (const string& s) {
auto info = wxLocale::FindLanguageInfo(U(s));
if (!info) {
cout << "Couldn't change locale to " << s << ", no locale information found" << endl;
return;
}
println("Changing language to %s...", U(info->CanonicalName));
locale = U(info->CanonicalName);
config.set("locale", locale);
initLocale();
initTranslations();
//todo: other consequences of changing locale
}

void App::OnQuit  () {
saveConfig();
}


