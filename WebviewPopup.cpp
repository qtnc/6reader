#include "WebViewPopup.hpp"
#include "wx/webview.h"
#include <wx/uiaction.h>
#include "MainWindow.hpp"
#include "app.hpp"

WebViewPopup::WebViewPopup (MainWindow* parent):
wxDialog(parent, wxID_ANY, "WebViewPopup", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX),
mainWindow(parent) {
webview = mainWindow->app.createWebView(this, wxID_ANY, U("about:blank"));
auto sizer = new wxBoxSizer(wxVERTICAL);
sizer->Add(webview, 1, wxEXPAND);
SetSizerAndFit(sizer);
Bind(wxEVT_INIT_DIALOG, [&](auto& e){ DoMagicClick(); });
webview->Bind(wxEVT_WEBVIEW_TITLE_CHANGED, [&](auto& e){ SetTitle(e.GetString()); });
webview->Bind(wxEVT_WEBVIEW_NEWWINDOW, [&](auto& e){ URLClicked(e.GetURL()); EndModal(wxOK); });
webview->Bind(wxEVT_WEBVIEW_NAVIGATING, [&](auto& e){ URLClicked(e.GetURL()); e.Veto(); EndModal(wxOK); });
webview->Bind(wxEVT_CHAR_HOOK, [&](auto& e){
if (e.GetModifiers()==0 && e.GetKeyCode()==WXK_ESCAPE) EndModal(wxOK);
e.Skip();
});
}

void WebViewPopup::SetPage (const wxString& str) {
webview->SetPage(str, U("about:blank"));
}

void WebViewPopup::DoMagicClick () {
webview->SetFocus();
wxUIActionSimulator bot;
auto position = webview->GetPosition();
position = webview->ClientToScreen(position);
position.x += 10; position.y += 10;
bot.MouseMove(position);
bot.MouseClick();
webview->SetFocus();
}

void WebViewPopup::URLClicked (const wxString& url) {
mainWindow->LoadURL(U(url));
}
