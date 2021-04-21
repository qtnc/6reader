#ifndef ___QXQSCRIPT_HPP___3
#define ___QXQSCRIPT_HPP___3
#include<string>
#include<unordered_map>

struct XQScript  {
std::string inputQuery, inputXML, baseURI;
std::string outputXML, outputError;
std::unordered_map<std::string,std::string> variables;
};

bool __declspec(dllexport) executeXQillaScript  (XQScript& xqs);
#endif
