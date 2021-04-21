#ifndef ___WEBVIEWPOPUP___1
#define ___WEBVIEWPOPUP___1
#include "WXWidgets.hpp"
#include<memory>

struct WebViewPopup: wxDialog {
struct wxWebView* webview;
struct MainWindow* mainWindow;


WebViewPopup (struct MainWindow* parent);
void SetPage (const wxString& html);
void DoMagicClick ();
void URLClicked (const wxString& url);
};

#endif
