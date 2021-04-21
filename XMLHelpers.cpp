#include "wxWidgets.hpp"
#include "XMLHelpers.hpp"
#include <wx/sstream.h>
#include<tidy.h>
#include<tidybuffio.h>
#include "cpprintf.hpp"
#include<memory>
using namespace std;

unique_ptr<wxXmlDocument> LoadHTML (const std::string& html) {
TidyBuffer out = {0, 0, 0, 0, 0};
TidyBuffer err = {0, 0, 0, 0, 0};
TidyDoc doc = tidyCreate();
finally ___f([=](){ tidyRelease(doc); });
tidyOptSetBool(doc, TidyXhtmlOut, yes);
tidySetErrorBuffer(doc, &err);
tidyParseString(doc, html.c_str());
tidyCleanAndRepair(doc);
tidyRunDiagnostics(doc);
tidySaveBuffer(doc, &out);
auto in = make_unique<wxStringInputStream>(U(out.bp));
auto xmldoc = make_unique<wxXmlDocument>();
if (!xmldoc->Load(*in)) xmldoc.reset();
return std::move(xmldoc);
}
