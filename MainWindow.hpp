#ifndef _____MAIN_WINDOW_0
#define _____MAIN_WINDOW_0
#include "WXWidgets.hpp"
#include<memory>
#include<functional>

struct MainWindow: wxFrame {
struct App& app;
struct wxWebView* webview;
struct wxNotebook* leftPane;
struct wxTreeCtrl* tocview;
struct wxListView* bookmarkList;
struct wxListView* spineList;
struct wxListView* resourceList;
struct wxListView* indexList;
struct wxListView* searchList;
struct wxTextCtrl* indexTerm;
struct wxTextCtrl* searchTerm;
struct wxPanel *indexPanel, *searchPanel;
struct wxSplitterWindow* splitter;
struct wxStatusBar* status;
struct wxToolBar* toolbar;
struct wxTimer* timer;
std::function<void()> timeoutFunc;
struct wxProgressDialog* progressDialog = nullptr;
bool progressCancelled=false;

std::shared_ptr<struct Book> book = nullptr;
std::shared_ptr<struct BookUserData> bookUdata = nullptr;
std::string jumpToBookmarkOnLoad;

MainWindow (App& app);
void showAboutBox (wxWindow* parent);

void openProgress (const std::string& title, const std::string& message, int maxValue);
void closeProgress ();
void updateProgress (int value);
void setProgressText (const std::string& msg);
void setProgressTitle (const std::string& msg);
void setProgressMax (int max);
bool isProgressCancelled ();
void OnProgress (struct wxThreadEvent& e);

int popupMenu (const std::vector<std::string>& items, int selection=-1);
int popupMenu (const std::vector<std::pair<int,std::string>>& items, int selection=-1);

void updateLeftPane ();
void updateTocView ();
void populateTocView ();
void populateBookmarkList ();
void populateSpineList ();
void populateResourceList ();
void populateIndexList ();
void updateTitle (const std::string& title);

void OnClose (wxCloseEvent& e);
void OnAction (wxCommandEvent& e);
void OnTocItemActivate (struct wxTreeEvent& e);
void OnBookmarkActivate (struct wxListEvent& e);
void OnBookmarkListContextMenu (struct wxContextMenuEvent& e);
void OnBookmarkKeyDown (struct wxKeyEvent& e);
void OnSpineItemActivate (struct wxListEvent& e);
void OnResourceItemActivate (struct wxListEvent& e);
void OnIndexItemActivate (struct wxListEvent& e);
void OnSearchItemActivate (struct wxListEvent& e);
void OnIndexTermEnter (wxCommandEvent& e);
void OnLeftPaneKeyDown (wxKeyEvent& e);
void OnWebViewCharHook (struct wxKeyEvent& e);
void OnWebViewContextMenu ();
void OnNavigating (struct wxWebViewEvent& e);
void OnNavigated (struct wxWebViewEvent& e);
void OnURLLoaded (struct wxWebViewEvent& e);
void OnTitleChanged (struct wxWebViewEvent& e);

bool DoAction (int id);
void DoMagicClick ();
void SetTimeout (int ms, const std::function<void()>& func);
void OpenFileDlg ();
void Quit ();
void GoBack ();
void GoForward ();
void GoNextPage ();
void GoPrevPage ();
void GoNextPane ();
void GoPrevPane ();
void AddBookmarkDlg ();
void JumpToBookmarkDlg ();
bool JumpToBookmark (int index);
bool JumpToPosition (const std::string& position);
void bookmarkMoveUp ();
void bookmarkMoveDown ();
void bookmarkDelete (bool skipConfirm=false);

std::string getCurrentURI ();
void LoadURI (const std::string& url);
void LoadURL (const std::string& url);
void CloseBook ();
bool LoadBook (const std::string& filename, const std::string& uriInBook = "");
void loadBookUserData ();
void saveBookUserData ();
std::string getBookUserDataFileName ();
};

#endif
