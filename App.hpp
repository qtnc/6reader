#ifndef _____APP_HPP_0
#define _____APP_HPP_0
#include "constants.h"
#include "PropertyMap.hpp"
#include "WXWidgets.hpp"
#include <wx/thread.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include<string>
#include<vector>
#include<unordered_map>
#include<fstream>
#include<functional>
#include "cpprintf.hpp"

struct App;
wxDECLARE_APP(App);

struct App: wxApp {
struct MainWindow* win = nullptr;
wxLocale* wxlocale;
std::string locale;

PropertyMap config, lang;
wxPathList pathList;
wxString appDir, userDir, userLocalDir;

wxString onLoadScript;
std::string fileToOpen, uriToOpen;
std::vector<std::shared_ptr<struct XQillaScript>> xqScripts;
struct WebViewBookHandler* bookFSHandler;
wxSharedPtr<struct wxWebViewHandler> bookFsHandlerSp;

bool initDirs ();
bool initConfig ();
bool initLocale ();
bool initTranslations ();
bool initWebview ();
bool initMisc ();
bool initScriptList ();

bool saveConfig ();
std::string findWritablePath (const std::string& filename);
void changeLocale (const std::string& s);
struct wxWebView* createWebView (wxWindow* parent, int id, const wxString& initialPage);

virtual bool OnInit () override;
virtual void OnInitCmdLine (wxCmdLineParser& cmd) override;
virtual bool OnCmdLineParsed (wxCmdLineParser& cmd) override;
void OnQuit ();
};

template <class F> inline void RunEDT (const F& f) {
wxGetApp().CallAfter(f);
}

#endif

