#ifndef ___XQILLASCRIPT_HPP___0
#define ___XQILLASCRIPT_HPP___0
#include<string>

#define XQS_INPUT_NONE 0
#define XQS_INPUT_SELECTION 1
#define XQS_INPUT_PROMPT 2

#define XQS_OUTPUT_PAGE 0
#define XQS_OUTPUT_SPEAK 1

#define XQS_FLAG_DOUBLE_UTF8 1

struct XQillaScript {
std::string file, name, icon, description;
int input = XQS_INPUT_SELECTION, output = XQS_OUTPUT_PAGE, flags=0;
int shortcutKey=0, shortcutModifiers=0;

bool load (const std::string& filename);
void execute (struct App& app, struct MainWindow* mainWindow, struct wxWebView* webview);
void showResult (const std::string& result, const std::string& error, bool success, struct App& app, struct MainWindow* mainWindow, struct wxWebView* webview);
void assignShortcut (const std::string& str);
std::string shortcutToString ();
bool valid ();
};

#endif
