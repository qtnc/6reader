#include "cpprintf.hpp"
#include "stringUtils.hpp"
#include "MainWindow.hpp"
#include "App.hpp"
#include "Book.hpp"
#include "BookUserData.hpp"
#include "WebViewBookHandler.hpp"
#include "XQillaScript.hpp"
#include "UniversalSpeech.h"
#include "WXWidgets.hpp"
#include <wx/listctrl.h>
#include <wx/thread.h>
#include <wx/notebook.h>
#include <wx/toolbar.h>
#include <wx/progdlg.h>
#include <wx/aboutdlg.h>
#include <wx/accel.h>
#include <wx/settings.h>
#include <wx/gbsizer.h>
#include <wx/timer.h>
#include <wx/scrolbar.h>
#include <wx/slider.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/textdlg.h>
#include <wx/choicdlg.h>
#include <wx/log.h>
#include <wx/uiaction.h>
#include <wx/wupdlock.h>
#include "wx/webview.h"
#include "wx/msw/webview_ie.h"
#include "wx/filesys.h"
#include "wx/fs_arc.h"
#include "wx/fs_mem.h"
#include<typeinfo>
using namespace std;

extern "C" void jfwRunFunctionA (const void* script);
extern "C" void jfwRunScriptA (const void* script);

struct TocEntryWrapper: wxTreeItemData {
shared_ptr<TocEntry> entry;
TocEntryWrapper (const shared_ptr<TocEntry>& te): entry(te) {}
};

wxBitmap loadBitmap (const wxString& fn) {
wxLogNull logNull;
wxImage image;
if (image.LoadFile(wxGetApp().appDir + "/" + fn)) {
wxBitmap bitmap(image);
return bitmap;
}
else {
wxBitmap bitmap;
bitmap.LoadFile(wxGetApp() .appDir + "/smile.xpm", wxBITMAP_TYPE_XPM);
return bitmap;
}}

MainWindow::MainWindow (App& app):
wxFrame(nullptr, wxID_ANY,
U(APP_DISPLAY_NAME),
wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE),
app(app)
{
println("Initializing main window GUI...");
//auto panel = new wxPanel(this);
splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

leftPane = new wxNotebook(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

tocview = new wxTreeCtrl(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxTR_HIDE_ROOT | wxTR_SINGLE);
tocview->SetName(U(translate("lblTocView")));

bookmarkList = new wxListView(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_EDIT_LABELS);
bookmarkList->AppendColumn(wxEmptyString);

spineList = new wxListView(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
spineList->AppendColumn(wxEmptyString);

resourceList = new wxListView(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxLC_REPORT | wxLC_SINGLE_SEL);
resourceList->AppendColumn(U(translate("reslistName")));
resourceList->AppendColumn(U(translate("reslistId")));
resourceList->AppendColumn(U(translate("reslistURI")));
resourceList->AppendColumn(U(translate("reslistType")));

indexPanel = new wxPanel(leftPane);
auto lblIndexTerm = new wxStaticText(indexPanel, wxID_ANY, U(translate("lblIndexTerm")), wxPoint(-2, -2), wxSize(1, 1) );
indexTerm = new wxTextCtrl(indexPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
indexList = new wxListView(indexPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
indexList->AppendColumn(wxEmptyString);

searchPanel = new wxPanel(leftPane);
auto lblSearchTerm = new wxStaticText(searchPanel, wxID_ANY, U(translate("lblSearchTerm")), wxPoint(-2, -2), wxSize(1, 1) );
searchTerm = new wxTextCtrl(searchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
searchList = new wxListView(searchPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
searchList->AppendColumn(wxEmptyString);

leftPane->AddPage(tocview, U(translate("tabToc")), false);
leftPane->AddPage(spineList, U(translate("tabSpine")), false);
leftPane->AddPage(indexPanel, U(translate("tabIndex")), false);
leftPane->AddPage(bookmarkList, U(translate("tabBookmarks")), false);
leftPane->AddPage(resourceList, U(translate("tabResources")), false);
leftPane->AddPage(searchPanel, U(translate("tabSearch")), false);

auto defaultPage = format("file://%s/lang/default_%s.html", U(app.appDir), app.locale.substr(0,2));
webview = app.createWebView(splitter, wxID_ANY, U(defaultPage));
status = CreateStatusBar(1);
toolbar = CreateToolBar(wxTB_HORIZONTAL);

toolbar->AddTool(IDM_OPENFILE, U(translate("OpenFileTB")), loadBitmap("openfile.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("OpenFile")) );
toolbar->AddSeparator();
toolbar->AddTool(IDM_PREVPAGE, U(translate("PrevPageTB")), loadBitmap("prevpage.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("PrevPage")) ); 
toolbar->AddTool(IDM_NEXTPAGE, U(translate("NextPageTB")), loadBitmap("nextpage.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("NextPage")) );
toolbar->AddTool(IDM_GOBACK, U(translate("GoBackTB")), loadBitmap("goback.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("GoBack")) ); 
toolbar->AddTool(IDM_GOFORWARD, U(translate("GoForwardTB")), loadBitmap("goforward.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("GoForward")) ); 
toolbar->AddSeparator();
toolbar->AddTool(IDM_JUMP_TO_BOOKMARK, U(translate("JumpToBookmarkTB")), loadBitmap("jumptobookmark.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("JumpToBookmark")) ); 
toolbar->AddTool(IDM_ADD_BOOKMARK, U(translate("AddBookmarkTB")), loadBitmap("addbookmark.xpm"), wxNullBitmap, wxITEM_NORMAL, U(translate("AddBookmark")) ); 
toolbar->AddSeparator();

auto sizer = new wxBoxSizer(wxVERTICAL);
sizer->Add(splitter, 1, wxEXPAND);
auto indexSizer = new wxBoxSizer(wxVERTICAL);
indexSizer->Add(indexTerm, 0, wxEXPAND);
indexSizer->Add(indexList, 1, wxEXPAND);
auto searchSizer = new wxBoxSizer(wxVERTICAL);
searchSizer->Add(searchTerm, 0, wxEXPAND);
searchSizer->Add(searchList, 1, wxEXPAND);

indexPanel->SetSizerAndFit(indexSizer);
searchPanel->SetSizerAndFit(searchSizer);
splitter->SplitVertically(leftPane, webview);

auto menubar = new wxMenuBar();
auto fileMenu = new wxMenu();
auto navMenu = new wxMenu();
auto toolsMenu = new wxMenu();
auto windowMenu = new wxMenu();
fileMenu->Append(IDM_OPENFILE, U(translate("OpenFile")));
//fileMenu->AppendSubMenu(openMenu, U(translate("OpenSubMenu")));
fileMenu->Append(wxID_EXIT, U(translate("Exit")));
navMenu->Append(IDM_NEXTPAGE, U(translate("NextPage")));
navMenu->Append(IDM_PREVPAGE, U(translate("PrevPage")));
navMenu->Append(IDM_GOBACK, U(translate("GoBack")));
navMenu->Append(IDM_GOFORWARD, U(translate("GoForward")));
navMenu->Append(IDM_ADD_BOOKMARK, translate("AddBookmark"));
navMenu->Append(IDM_JUMP_TO_BOOKMARK, translate("JumpToBookmark"));
//windowMenu->AppendCheckItem(IDM_SHOWPLAYLIST, U(translate("Playlist")));
menubar->Append(fileMenu, U(translate("File")));
menubar->Append(navMenu, U(translate("Navigate")));
menubar->Append(toolsMenu, U(translate("Tools")));
menubar->Append(windowMenu, U(translate("Window")));
SetMenuBar(menubar);

timer = new wxTimer(this);
Bind(wxEVT_TIMER, [&](auto& e){ timeoutFunc(); });
webview->Bind(wxEVT_WEBVIEW_TITLE_CHANGED, &MainWindow::OnTitleChanged, this);
webview->Bind(wxEVT_WEBVIEW_NAVIGATING, &MainWindow::OnNavigating, this);
webview->Bind(wxEVT_WEBVIEW_NAVIGATED, &MainWindow::OnNavigated, this);
webview->Bind(wxEVT_WEBVIEW_LOADED, &MainWindow::OnURLLoaded, this);
webview->Bind(wxEVT_CONTEXT_MENU, [&](auto& e){ OnWebViewContextMenu(); });
webview->Bind(wxEVT_WEBVIEW_ERROR, [&](auto& e){ println("Navigation error: url=%s, string=%s, value=%s", e.GetURL(), e.GetString(), e.GetNavigationAction() ); });
webview->Bind(wxEVT_WEBVIEW_NEWWINDOW, [&](auto& e){ println("New window: target=%s, url=%s, navigation action = %d", e.GetTarget(), e.GetURL(), e.GetNavigationAction() ); });
webview->Bind(wxEVT_CHAR_HOOK, &MainWindow::OnWebViewCharHook, this);
tocview->Bind(wxEVT_TREE_ITEM_ACTIVATED, &MainWindow::OnTocItemActivate, this);
tocview->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
bookmarkList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MainWindow::OnBookmarkActivate, this);
bookmarkList->Bind(wxEVT_LIST_END_LABEL_EDIT, [&](auto& e){ bookUdata->bookmarks[e.GetIndex()]->name = U(e.GetLabel()); populateBookmarkList(); });
bookmarkList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnBookmarkKeyDown, this);
bookmarkList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
bookmarkList->Bind(wxEVT_CONTEXT_MENU, &MainWindow::OnBookmarkListContextMenu, this);
spineList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MainWindow::OnSpineItemActivate, this);
spineList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
resourceList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MainWindow::OnResourceItemActivate, this);
resourceList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
indexList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MainWindow::OnIndexItemActivate, this);
indexList->Bind(wxEVT_KEY_DOWN, [&](auto& e){ if (e.GetModifiers()==wxMOD_NONE && e.GetKeyCode()==WXK_ESCAPE) { indexTerm->SetFocus(); } else e.Skip(); });
indexList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
indexTerm->Bind(wxEVT_TEXT, [&](auto& e){ SetTimeout(500, [&](){ populateIndexList(); }); });
indexTerm->Bind(wxEVT_TEXT_ENTER, &MainWindow::OnIndexTermEnter, this);
indexTerm->Bind(wxEVT_KEY_DOWN, [&](auto& e){ if (e.GetModifiers()==wxMOD_NONE && e.GetKeyCode()==WXK_DOWN && indexList->GetItemCount()>0) { indexList->Select(0); indexList->Focus(0); indexList->SetFocus(); } e.Skip(); });
indexTerm->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
indexTerm->Bind(wxEVT_SET_FOCUS, [&](auto& e){ indexTerm->SelectAll(); });
searchList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &MainWindow::OnSearchItemActivate, this);
searchList->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
searchTerm->Bind(wxEVT_KEY_DOWN, &MainWindow::OnLeftPaneKeyDown, this);
leftPane->Bind(wxEVT_SET_FOCUS, [&](auto& e){ auto curPage = leftPane->GetCurrentPage(); if (curPage) curPage->SetFocus(); });
leftPane->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, [&](auto& e){ speechSayU(U(leftPane->GetPageText(e.GetSelection())).c_str(), true); });
Bind(wxEVT_MENU, MainWindow::OnAction, this);
//Bind(wxEVT_MENU, &MainWindow::OnOpenFile, this, IDM_OPENFILE);
//Bind(wxEVT_HOTKEY, &MainWindow::OnPlayPauseHK, this, IDM_PLAYPAUSE);
Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnClose, this);
//Bind(wxEVT_ACTIVATE_APP, [](wxActivateEvent& e){ println("Activate app %s", e.GetActive()); e.Skip(); });
//Bind(wxEVT_ACTIVATE, [&](wxActivateEvent& e){ active=e.GetActive();  e.Skip(); });
//Bind(wxEVT_ICONIZE, [&](wxIconizeEvent& e){ active=!e.IsIconized();  e.Skip(); });
Bind(wxEVT_THREAD, &MainWindow::OnProgress, this);


println("Init toolbar with %d buttons", app.xqScripts.size());
for (int i=0, n=app.xqScripts.size(); i<n; i++) {
auto& script = app.xqScripts[i];
if (!script->name.empty()) {
auto label = U(translate(script->name));
auto description = script->description.empty()? label : U(translate(script->description));
int id = IDM_XQSCRIPT +i;
toolsMenu->Append(id, label);
if (!script->icon.empty()) {
auto bitmap = loadBitmap(U(script->icon));
toolbar->AddTool(id, label, bitmap, wxNullBitmap, wxITEM_NORMAL, label, description);
}}}

toolbar->Realize();
SetSizerAndFit(sizer);
SetSize(wxSize(800, 600));

vector<wxAcceleratorEntry> entries = {
{ wxACCEL_CTRL, WXK_PAGEUP, IDM_PREVPAGE },
{ wxACCEL_CTRL, WXK_PAGEDOWN, IDM_NEXTPAGE },
{ wxACCEL_NORMAL, WXK_F6, IDM_NEXTPANE },
{ wxACCEL_SHIFT, WXK_F6, IDM_PREVPANE }
};
wxAcceleratorTable table(entries.size(), &entries[0]);
SetAcceleratorTable(table);

Show(true);
SetFocus();
DoMagicClick();

LoadBook(app.fileToOpen, app.uriToOpen);
println("Initialized main window GUI");
}

int MainWindow::popupMenu (const vector<string>& items, int selection) {
wxMenu menu;
for (int i=0, n=items.size(); i<n; i++) menu.Append(i+1, U(items[i]), wxEmptyString, selection<0? wxITEM_NORMAL : wxITEM_RADIO);
if (selection>=0) menu.Check(selection+1, true);
int result = GetPopupMenuSelectionFromUser(menu);
return result==wxID_NONE? -1 : result -1;
}

int MainWindow::popupMenu (const vector<pair<int,string>>& items, int selection) {
wxMenu menu;
for (auto& item: items) menu.Append(item.first, U(item.second), wxEmptyString, selection<0? wxITEM_NORMAL : wxITEM_RADIO);
if (selection>0) menu.Check(selection, true);
return GetPopupMenuSelectionFromUser(menu);
}

void MainWindow::openProgress (const std::string& title, const std::string& message, int maxValue) {
RunEDT([=](){
progressCancelled=false;
progressDialog = new wxProgressDialog(U(title), U(message), maxValue, this, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
});
}

void MainWindow::closeProgress () {
progressDialog->Destroy();
progressDialog=nullptr;
wxWakeUpIdle();
}

void MainWindow::updateProgress (int value) {
wxThreadEvent e(wxEVT_THREAD, 9999);
e.SetInt(value);
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressMax (int value) {
wxThreadEvent e(wxEVT_THREAD, 9997);
e.SetInt(value);
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressText (const std::string& msg) {
wxThreadEvent e(wxEVT_THREAD, 9998);
e.SetString(U(msg));
wxQueueEvent(this, e.Clone());
}

void MainWindow::setProgressTitle (const std::string& msg) {
wxThreadEvent e(wxEVT_THREAD, 9996);
e.SetString(U(msg));
wxQueueEvent(this, e.Clone());
}

void MainWindow::OnProgress (wxThreadEvent& e) {
switch(e.GetId()) {
case 9999: {
int value = e.GetInt();
if (progressDialog) progressCancelled = !progressDialog->Update(value);
}break;
case 9998: {
wxString value = e.GetString();
if (progressDialog) progressCancelled = !progressDialog->Update(progressDialog->GetValue(), value);
}break;
case 9997: {
int value = e.GetInt();
if (progressDialog) progressDialog->SetRange(value);
}break;
case 9996: {
wxString value = e.GetString();
if (progressDialog) progressDialog->SetTitle(value);
}break;
}}

bool MainWindow::isProgressCancelled () { return progressCancelled; }

void MainWindow::showAboutBox (wxWindow* parent) {
string name = APP_DISPLAY_NAME;
wxAboutDialogInfo info;
info.SetCopyright("Copyright (C) 2019, QuentinC");
info.SetName(name);
info.SetVersion(format("%1.%2.%3", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD));
info.SetWebSite("https://quentinc.net/");
wxAboutBox(info, parent);
}

void MainWindow::SetTimeout (int ms, const std::function<void()>& func) {
timeoutFunc = func;
timer->StartOnce(ms);
}

void MainWindow::DoMagicClick () {
leftPane->SetFocus();
wxUIActionSimulator bot;
auto position = webview->GetPosition();
position = webview->ClientToScreen(position);
position.x += 10; position.y += 10;
bot.MouseMove(position);
bot.MouseClick();
webview->SetFocus();
}

void MainWindow::loadBookUserData () {
string fn = getBookUserDataFileName();
if (book && fn.size()) {
bookUdata = make_shared<BookUserData>(book);
bookUdata->load(fn);
}}

void MainWindow::saveBookUserData () {
string fn = getBookUserDataFileName();
if (book && bookUdata && fn.size()) {
bookUdata->save(fn);
}}

std::string MainWindow::getBookUserDataFileName () {
if (!book) return "";
return format("%s/%s.xml", U(app.userDir), book->getHashName());
}


bool MainWindow::LoadBook (const string& filename, const string& lastURI) {
try {
println("Loading book: %s, %s", filename, lastURI);
if (filename.empty()) return false;
wxLogNull logNull;
auto newBook = Book::open(filename);
if (!newBook) {
wxMessageBox(U(format(translate("msgOpenBookFail"), filename)), U(translate("appName")), wxOK | wxICON_ERROR);
return false;
}
app.bookFSHandler->removeBook(book);
app.bookFSHandler->addBook(newBook);
book = newBook;
loadBookUserData();
updateLeftPane();

string uri = lastURI;
if (bookUdata && bookUdata->lastUri.size()) {
uri = bookUdata->lastUri;
jumpToBookmarkOnLoad = bookUdata->lastPosition;
}
if (uri.empty()) uri = book->getFirstPageURI();
LoadURI(uri);
return true;
} catch (exception& e) {
println("Load book failed: %s: %s", typeid(e).name(), e.what());
string msg = format(translate("msgOpenBookFail"), filename);
msg += format("\r\n%s: %s", typeid(e).name(), e.what());
wxMessageBox(U(msg), U(translate("appName")), wxOK | wxICON_ERROR);
return false;
} }

void MainWindow::CloseBook () {
if (!book) return;
if (webview) {
wxString uPos;
webview->RunScript("window.___epubGetCurrentPosition()", &uPos);
bookUdata->lastUri = getCurrentURI();
bookUdata->lastPosition = U(uPos);
}
saveBookUserData();
}

void MainWindow::updateTitle (const string& title) {
auto appTitle = app.lang.get("appTitle", "%s");
if (starts_with(title, "file://") || starts_with(title, "wxfs://")) appTitle = format(appTitle, title.substr(title.rfind('/')));
else appTitle = format(appTitle, title);
SetTitle(U(appTitle));
}

void MainWindow::updateLeftPane () {
if (!book) return;
int flags = book->getFlags();
populateTocView();
populateBookmarkList();
populateSpineList();
populateResourceList();
indexTerm->SetValue(wxEmptyString);
indexList->DeleteAllItems();
searchTerm->SetValue(wxEmptyString);
searchList->DeleteAllItems();

webview->SetFocus();
wxWindowUpdateLocker lpUpdateLocker(leftPane);
int pageCount = leftPane->GetPageCount();
if (pageCount>0) for (int i=pageCount-1; i>=0; i--) leftPane->RemovePage(i);

if (flags&BOOK_HAS_TOC) leftPane->AddPage(tocview, U(translate("tabToc")), false);
if (flags&BOOK_HAS_SPINE) leftPane->AddPage(spineList, U(translate("tabSpine")), false);
if (flags&BOOK_HAS_INDEX) leftPane->AddPage(indexPanel, U(translate("tabIndex")), false);
if (flags&BOOK_HAS_BOOKMARKS) leftPane->AddPage(bookmarkList, U(translate("tabBookmarks")), false);
if (flags&BOOK_HAS_RESOURCES) leftPane->AddPage(resourceList, U(translate("tabResources")), false);
if (flags&BOOK_HAS_SEARCH) leftPane->AddPage(searchPanel, U(translate("tabSearch")), false);

pageCount = leftPane->GetPageCount();
bool isSplit = splitter->IsSplit();
wxWindow* page0 = pageCount>0? leftPane->GetPage(0) : nullptr;
if (pageCount>0) {
if (!isSplit) splitter->SplitVertically(leftPane, webview);
leftPane->ChangeSelection(0);
webview->SetFocus();
}
else {
if (isSplit) splitter->Unsplit(leftPane);
webview->SetFocus();
}}

void populateTocViewSubtree (wxTreeCtrl* tocview, wxTreeItemId root, shared_ptr<TocEntry> toc) {
if (!toc) return;
for (auto item: toc->children) {
auto itemWrapper = new TocEntryWrapper(item);
auto child = tocview->AppendItem(root, U(item->label), -1, -1, itemWrapper);
item->treeItem = itemWrapper;
populateTocViewSubtree(tocview, child, item);
}}

void MainWindow::populateTocView () {
wxWindowUpdateLocker updateLocker(tocview);
tocview->DeleteAllItems();
if (!book) return;
auto toc = book->getToc();
if (!toc) return;
auto root = tocview->AddRoot("#Root");
populateTocViewSubtree(tocview, root, toc);
}

void MainWindow::populateBookmarkList () {
wxWindowUpdateLocker updateLocker(bookmarkList);
int curSel = bookmarkList->GetFirstSelected();
bookmarkList->DeleteAllItems();
if (bookUdata && bookUdata->bookmarks.size()) {
int index = 0;
for (auto& bm: bookUdata->bookmarks) {
bookmarkList->InsertItem(index++, U(bm->name));
}}
if (curSel>=0 && curSel<bookmarkList->GetItemCount()) {
bookmarkList->Select(curSel);
bookmarkList->Focus(curSel);
}}

void MainWindow::populateSpineList () {
wxWindowUpdateLocker updateLocker(spineList);
int curSel = spineList->GetFirstSelected();
spineList->DeleteAllItems();
int index=-1;
for (auto item: book->getSpine()) {
if (item->name.empty()) continue;
spineList->InsertItem(++index, U(item->name));
spineList->SetItemPtrData(index, item->uidptr);
}
if (curSel>=0 && curSel<spineList->GetItemCount()) {
spineList->Select(curSel);
spineList->Focus(curSel);
}}

void MainWindow::populateResourceList () {
wxWindowUpdateLocker updateLocker(resourceList);
int curSel = resourceList->GetFirstSelected();
resourceList->DeleteAllItems();
int index=-1;
for (auto item: book->getResources()) {
auto& name = item->name.size()? item->name : (item->id.size()? item->id : item->uri);
resourceList->InsertItem(++index, U(name));
resourceList->SetItem(index, 1, U(item->id));
resourceList->SetItem(index, 2, U(item->uri));
resourceList->SetItem(index, 3, U(item->type));
resourceList->SetItemPtrData(index, item->uidptr);
}
if (curSel>=0 && curSel<spineList->GetItemCount()) {
spineList->Select(curSel);
spineList->Focus(curSel);
}}

void MainWindow::populateIndexList () {
wxWindowUpdateLocker updateLocker(indexList);
indexList->DeleteAllItems();
int index=-1;
string term = U(indexTerm->GetValue());
trim(term);
if (term.empty()) return;
auto results = book->findResourcesInIndex(term);
speechSayU(format(translate("msgNSearchResults"), results.size()).c_str(), true);
indexList->Hide();
for (auto item: results) {
if (item->name.empty()) continue;
indexList->InsertItem(++index, U(item->name));
indexList->SetItemPtrData(index, item->uidptr);
}
indexList->Show();
}

void MainWindow::OnTitleChanged (wxWebViewEvent& e) {
auto title = U(e.GetString());
if (title=="internal:context-menu") OnWebViewContextMenu();
else updateTitle(title);
}

void MainWindow::OnNavigating (wxWebViewEvent& e) {
auto url = U(e.GetURL());
if (starts_with(url, "internal:")) {
e.Veto();
if (url=="internal:context-menu") OnWebViewContextMenu();
}
else e.Skip();
}

void MainWindow::OnNavigated (wxWebViewEvent& e) {
updateTocView();
}

void MainWindow::OnURLLoaded (wxWebViewEvent& e) {
if (app.onLoadScript.size()) webview->RunScript(app.onLoadScript);
if (jumpToBookmarkOnLoad.size()) {
if (JumpToPosition(jumpToBookmarkOnLoad)) jumpToBookmarkOnLoad.clear();
}
}

void MainWindow::OnLeftPaneKeyDown (wxKeyEvent& e) {
int mod = e.GetModifiers(), k=e.GetKeyCode();
if (mod==wxMOD_NONE && k==WXK_ESCAPE) GoNextPane();
else e.Skip();
}

void MainWindow::OnTocItemActivate (wxTreeEvent& e) {
auto itemId = e.GetItem();
auto itemData = tocview->GetItemData(itemId);
if (!itemData) return;
auto entry = static_cast<TocEntryWrapper*>(itemData) ->entry;
if (entry->uri.empty()) { wxBell(); return; }
LoadURI(entry->uri);
}

void MainWindow::OnSpineItemActivate (wxListEvent& e) {
int index = e.GetIndex();
auto uidptr = spineList->GetItemData(index);
if (!uidptr) { wxBell(); return; }
string& uri = *reinterpret_cast<string*>(uidptr);
LoadURI(uri);
}

void MainWindow::OnResourceItemActivate (wxListEvent& e) {
int index = e.GetIndex();
auto uidptr = resourceList->GetItemData(index);
if (!uidptr) { wxBell(); return; }
string& uri = *reinterpret_cast<string*>(uidptr);
LoadURI(uri);
}

void MainWindow::OnIndexItemActivate (wxListEvent& e) {
int index = e.GetIndex();
auto uidptr = indexList->GetItemData(index);
if (!uidptr) { wxBell(); return; }
string& uri = *reinterpret_cast<string*>(uidptr);
if (indexList->GetFirstSelected()==0) indexTerm->SetFocus();
LoadURI(uri);
}

void MainWindow::OnSearchItemActivate (wxListEvent& e) {
int index = e.GetIndex();
auto uidptr = searchList->GetItemData(index);
if (!uidptr) { wxBell(); return; }
string& uri = *reinterpret_cast<string*>(uidptr);
LoadURI(uri);
}

void MainWindow::OnBookmarkActivate (wxListEvent& e) {
JumpToBookmark(e.GetIndex());
}

void MainWindow::OnBookmarkListContextMenu (wxContextMenuEvent& e) {
int action = popupMenu({ translate("ctxGoTo"), translate("ctxRename"), translate("ctxMoveUp"), translate("ctxMoveDown"), translate("ctxDelete") });
int i = bookmarkList->GetFirstSelected();
switch(action){
case 0: JumpToBookmark(i); break;
case 1: bookmarkList->EditLabel(i); break;
case 2: bookmarkMoveUp(); break;
case 3: bookmarkMoveDown(); break;
case 4: bookmarkDelete(); break;
default: break;
}}

void MainWindow::OnBookmarkKeyDown (wxKeyEvent& e) {
int k = e.GetKeyCode(), mod = e.GetModifiers(), i = bookmarkList->GetFirstSelected();
if (k==WXK_F2 && mod==wxMOD_NONE) bookmarkList->EditLabel(i);
else if (k==WXK_DELETE) bookmarkDelete(mod&wxMOD_SHIFT);
else if (k==WXK_UP && mod==wxMOD_SHIFT && i>0) bookmarkMoveUp();
else if (k==WXK_DOWN && mod==wxMOD_SHIFT && i<bookUdata->bookmarks.size() -1) bookmarkMoveDown();
else e.Skip();
}

void MainWindow::OnIndexTermEnter (wxCommandEvent& e) {
timer->Stop();
populateIndexList();
if (indexList->GetItemCount()>0) {
auto uidptr = indexList->GetItemData(0);
if (!uidptr) { wxBell(); return; }
string& uri = *reinterpret_cast<string*>(uidptr);
LoadURI(uri);
}
else wxBell();
}

void MainWindow::OnAction (wxCommandEvent& e) {
if (!DoAction(e.GetId())) e.Skip();
}

bool MainWindow::DoAction (int id) {
if (id>=IDM_XQSCRIPT && id<=IDM_XQSCRIPT_MAX) {
id -= IDM_XQSCRIPT;
if (id>=0 && id<app.xqScripts.size()) app.xqScripts[id]->execute(app, this, webview);
}
switch(id) {
case IDM_OPENFILE: OpenFileDlg(); break;
case IDM_GOBACK: GoBack(); break;
case IDM_GOFORWARD: GoForward(); break;
case IDM_NEXTPAGE: GoNextPage(); break;
case IDM_PREVPAGE: GoPrevPage(); break;
case IDM_NEXTPANE: GoNextPane(); break;
case IDM_PREVPANE: GoPrevPane(); break;
case IDM_ADD_BOOKMARK: AddBookmarkDlg(); break;
case IDM_JUMP_TO_BOOKMARK: JumpToBookmarkDlg(); break;
case wxID_EXIT: Close(); break;
default: return false;
}
return true;
}

void MainWindow::OnWebViewContextMenu () {
bool hasSelection = webview->HasSelection();
vector<pair<int,string>> options;
if (hasSelection) {
initializer_list<pair<int,string>> options2 = {
{ wxID_COPY, translate("ctxCopy") }, 
{ wxID_CUT, translate("ctxCut") }, 
{ wxID_PASTE, translate("ctxPaste") }
};
options.insert(options.end(), options2.begin(), options2.end());
}
if (!webview->IsEditable()) {
initializer_list<pair<int,string>> options2 = {
{ IDM_GOBACK, translate("GoBack") }, 
{ IDM_GOFORWARD, translate("GoForward") },
{ IDM_PREVPAGE, translate("PrevPage") }, 
{ IDM_NEXTPAGE, translate("NextPage") },
{ IDM_JUMP_TO_BOOKMARK, translate("JumpToBookmark") },
{ IDM_ADD_BOOKMARK, translate("AddBookmark") }
};
options.insert(options.end(), options2.begin(), options2.end());
for (int i=0, n=app.xqScripts.size(); i<n; i++) {
auto& xq = app.xqScripts[i];
if (xq->name.empty()) continue;
if (xq->input==XQS_INPUT_SELECTION && !hasSelection) continue;
string name;
if (xq->shortcutKey) name = xq->name + "\t" + xq->shortcutToString();
else name = xq->name;
options.push_back(make_pair(IDM_XQSCRIPT +i, name));
}
}
if (options.empty()) return;
int action = popupMenu(options);
if (action>0) DoAction(action);
}

void runXQScript (const string& xqrootdir, const string& xqfn, const string& arg);
void MainWindow::OnWebViewCharHook (wxKeyEvent& e) {
int mod = e.GetModifiers(), k = e.GetKeyCode();
if (mod==wxMOD_NONE) switch(k) {
case WXK_ESCAPE: GoPrevPane(); break;
case WXK_WINDOWS_MENU: OnWebViewContextMenu(); break;
}
else if (mod==wxMOD_CONTROL) switch(k){
case 'O': OpenFileDlg(); break;
case 'J': JumpToBookmarkDlg(); break;
case 'K': AddBookmarkDlg(); break;
case 'E': webview->SetEditable(!webview->IsEditable()); DoMagicClick(); break;
//case 'F': jfwRunFunctionA("FindString(GetCurrentWindow(), \"access\", S_BOTTOM, S_RESTRICTED, true);"); break;
}
if (mod&wxMOD_CONTROL) for (auto& xqs: app.xqScripts) {
if (xqs->shortcutKey==k && xqs->shortcutModifiers==mod) {
xqs->execute(app, this, webview);
break;
}}
e.Skip();
}

std::string buildFilenameFilters (function<string(const string&)> trans) {
auto& pairs = Book::getOpenFileFilters();
string allfiles = "*.*", allexts;
vector<string> filters, exts;
for (auto& p: pairs) {
exts.push_back(p.second);
filters.push_back(format("%s (%s)|%s", p.first, p.second, p.second));
}
allexts = join(exts, ";");
filters.insert(filters.begin(), format("%s (%s)|%s", trans("AllSupportedFiles"), allexts, allexts));
filters.push_back(format("%s (%s)|%s", trans("AllFiles"), allfiles, allfiles));
return join(filters, "|");
}

void MainWindow::OpenFileDlg () {
wxString wildcardFilter = U(buildFilenameFilters([&](auto& t){ return translate(t); })); //wxFileSelectorDefaultWildcardStr;
wxFileDialog fd(this, U(translate("OpenFileDlg")), wxEmptyString, wxEmptyString, wildcardFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
fd.SetFilterIndex(0);
if (wxID_OK==fd.ShowModal()) {
string filename = U(fd.GetPath());
CloseBook();
LoadBook(filename);
}
}

void MainWindow::GoBack () {
if (webview->CanGoBack()) webview->GoBack();
else wxBell();
if (tocview->HasFocus()) webview->SetFocus();
}

void MainWindow::GoForward () {
if (webview->CanGoForward()) webview->GoForward();
else wxBell();
if (tocview->HasFocus()) webview->SetFocus();
}

void MainWindow::GoNextPage () {
string uri = urldecode(getCurrentURI());
uri = book->getNextPageURI(uri);
if (uri.empty()) { wxBell(); return; }
LoadURI(uri);
}

void MainWindow::GoPrevPage () {
string uri = urldecode(getCurrentURI());
uri = book->getPrevPageURI(uri);
if (uri.empty()) { wxBell(); return; }
LoadURI(uri);
}

bool hasAncestor (wxWindow* w, wxWindow* p) {
do {
if (w==p) return true;
} while(w && (w=w->GetParent()));
return false;
}

void MainWindow::GoNextPane () {
if (!splitter->IsSplit()) return;
wxWindow* toFocus = nullptr;
auto curFocus = wxWindow::FindFocus();
if (hasAncestor(curFocus, leftPane)) toFocus = webview;
else {
speechSayU(U(leftPane->GetPageText(leftPane->GetSelection())).c_str(), true); 
toFocus = leftPane; 
}
if (toFocus) toFocus->SetFocus();
}


void MainWindow::GoPrevPane () {
GoNextPane();
}

string removeHashPart (const string& s);

void MainWindow::AddBookmarkDlg () {
if (!webview || !webview->HasSelection()) {
wxMessageBox(U(translate("msgBookmarkNoSelection")), U(translate("addBookmarkCaption")), wxICON_INFORMATION | wxOK);
return;
}
string uri = getCurrentURI();
if (uri.empty()) return;
wxString uPos;
string bookmarkName = U(webview->GetSelectedText());
webview->RunScript("window.___epubGetCurrentPosition();", &uPos);
if (uPos.empty()) return;
if (bookmarkName.size()>50) bookmarkName = bookmarkName.substr(0, 50);
trim(bookmarkName);
bookmarkName = U(wxGetTextFromUser(U(translate("addBookmarkText")), U(translate("addBookmarkCaption")), U(bookmarkName), this));
trim(bookmarkName);
if (bookmarkName.empty()) return;
auto newBookmark = make_shared<Bookmark>(bookmarkName, uri, U(uPos));
auto oldBookmark = std::find_if(bookUdata->bookmarks.begin(), bookUdata->bookmarks.end(), [&](auto& b){ return iequals(b->name, bookmarkName); });
if (oldBookmark==bookUdata->bookmarks.end()) bookUdata->bookmarks.push_back(newBookmark);
else if (wxYES==wxMessageBox(U(translate("msgBookmarkNameExists")), U(translate("addBookmarkCaption")), wxICON_EXCLAMATION | wxYES_NO)) *oldBookmark = newBookmark;
populateBookmarkList();
}

void MainWindow::JumpToBookmarkDlg () {
auto& bookmarks = bookUdata->bookmarks;
if (bookmarks.empty()) return;
wxArrayString choices;
for (auto& bm: bookmarks) choices.push_back(U(bm->name));
int i = wxGetSingleChoiceIndex(U(translate("jumpToBookmarkText")), U(translate("jumpToBookmarkCaption")), choices, this);
JumpToBookmark(i);
}

bool MainWindow::JumpToBookmark (int i) {
if (!bookUdata) return false;
auto& bookmarks = bookUdata->bookmarks;
if (i<0 || i>=bookmarks.size()) return false;
auto& bookmark = bookmarks[i];
jumpToBookmarkOnLoad = bookmark->position;
LoadURI(bookmark->uri);
return true;
}

bool MainWindow::JumpToPosition (const string& position) {
if (!webview) return false;
wxString result;
return webview->RunScript(U(format("window.___epubJumpToPosition('%s');", position)), &result) && result=="true";
}

void MainWindow::bookmarkDelete (bool skipConfirm) {
int i = bookmarkList->GetFirstSelected();
if (skipConfirm || wxYES==wxMessageBox(U(format(translate("msgDeleteBookmark"), bookUdata->bookmarks[i]->name)), U(translate("tabBookmarks")), wxICON_EXCLAMATION | wxYES_NO)) {
bookUdata->bookmarks.erase(bookUdata->bookmarks.begin() + i);
populateBookmarkList();
}}


void MainWindow::bookmarkMoveUp () {
int i = bookmarkList->GetFirstSelected();
if (i<1) return;
std::swap(bookUdata->bookmarks[i], bookUdata->bookmarks[i-1]);
populateBookmarkList();
bookmarkList->Select(--i);
bookmarkList->Focus(i);
}

void MainWindow::bookmarkMoveDown () {
int i = bookmarkList->GetFirstSelected();
if (i<0 || i>=bookUdata->bookmarks.size() -1) return;
std::swap(bookUdata->bookmarks[i], bookUdata->bookmarks[i+1]);
populateBookmarkList();
bookmarkList->Select(++i);
bookmarkList->Focus(i);
}

void MainWindow::LoadURI (const string& uri) {
if (!book) return;
LoadURL(format("book:%s!/%s", book->getFileName(), uri));
}

void MainWindow::LoadURL (const string& url) {
println("Loading URL: %s", url);
webview->LoadURL(U(url));
auto curFocus = wxWindow::FindFocus();
if (hasAncestor(curFocus, leftPane)) webview->SetFocus();
}

string MainWindow::getCurrentURI () {
if (!webview) return "";
string url = U(webview->GetCurrentURL());
auto i = url.find("!/");
if (i==string::npos) return "";
return url.substr(i+2);
}

shared_ptr<TocEntry> updateTocViewSelection (wxTreeCtrl* tocview, shared_ptr<TocEntry> toc, const string& uri) {
if (toc->uri.size() && uri.size() && removeHashPart(uri) == removeHashPart(toc->uri)) {
auto treeItem = toc->treeItem;
if (treeItem) tocview->SelectItem(treeItem->GetId());
return toc;
}
shared_ptr<TocEntry> re = nullptr;
for (auto item: toc->children) if (re=updateTocViewSelection(tocview, item, uri)) return re;
return re;
}

void MainWindow::updateTocView () {
if (!book) return;
auto toc = book->getToc();
if (!toc) return;
auto uri = getCurrentURI();
if (uri.empty()) return;
auto entry = updateTocViewSelection(tocview, toc, uri);
if (entry) {
vector<string> tocPath;
for (auto e = entry; e; e=e->parent.lock()) if (e->label.size()) tocPath.push_back(e->label);
reverse(tocPath.begin(), tocPath.end());
string tocPathStat = join(tocPath, "/");
status->SetLabel(U(tocPathStat));
}
}

void MainWindow::Quit () {
if (book) app.config.set("book.last.filename", book->getFileName());
CloseBook();
app.OnQuit();
}

void MainWindow::OnClose (wxCloseEvent& e) {
//if (e.CanVeto() && app.config.get("general.confirmOnQuit", true) && wxNO==wxMessageBox(U(translate("confirmquit")), GetTitle(), wxICON_EXCLAMATION | wxYES_NO)) e.Veto();
//else 
Quit();
e.Skip();
}

