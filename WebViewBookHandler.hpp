#ifndef ___WEBVIEWWEBVIEWHANDLER___1
#define ___WEBVIEWWEBVIEWHANDLER___1
#include "WXWidgets.hpp"
#include <wx/webview.h>
#include<memory>
#include<unordered_map>

struct WebViewBookHandler: wxWebViewHandler {
std::unordered_map<std::string, std::shared_ptr<struct Book>> books;
std::unordered_map<std::string, std::pair<std::string, int>> cache;
size_t cacheSize = 0, maxCacheSize = 30 * 1024 * 1024; // 30 MB

WebViewBookHandler ();
wxFSFile* GetFile (const wxString& uri) override;
void addBook (const std::shared_ptr<Book>& b);
void removeBook (const std::shared_ptr<Book>& b);
void drainCache ();
};

#endif
