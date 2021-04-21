#ifndef _____BOOK_ZIP_HPP_0
#define _____BOOK_ZIP_HPP_0
#include "Book.hpp"
#include<string>
#include<vector>
#include<memory>
#include<functional>
#include <wx/stream.h>

class BookZip: public Book  {
protected:
std::shared_ptr<TocEntry> toc = nullptr;
bool isFolder = false;

public:
BookZip () {}
virtual bool load (const std::string& filename);

virtual std::shared_ptr<TocEntry> getToc () override { return toc; }
std::unique_ptr<wxInputStream> openResource (const std::shared_ptr<Resource>& resource) override;

~BookZip () override {}

virtual std::unique_ptr<wxInputStream> openFile (const std::string& fileNameInArchive);
virtual std::unique_ptr<wxInputStream> openFile (const std::function<bool(const std::string&)>& matcher);
};

std::string makeRelativePath (const std::string& relativePath, const std::string& absolutePathMakeRelativeTo);
std::string makeAbsolutePath (std::string filename);

#endif
