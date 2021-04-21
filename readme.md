# 6reader
6reader is an application to read epub and epub3 books in a very simple user interface, fully accessible to screen readers.

# Features

- Read books in a simple web view
- Simple and quick access to table of contents, spine, index and resource list if the book has them
- Ability to define named bookmarks
- Scripting with XQuilla, allowing for example to quickly look at word definition in dictionary without closing your book

6reader currently support reading books in the following formats:

- epub, epub 2.0 and 3.0
- Compressed help (CHM), only under windows and if HtmlWorkshop is installed
- Dictionary files from DIC application
- HTML
- Plain text

# History
To read books on mobile phone, I can recommand the excellent VoiceDream Reader.
However, in documentations, scientific, school, history, cooking or whatever other book that isn't fiction, reading may not be linear: you need to bookmark, jump ahead, come back, read again, lookup references, etc.
IN all these cases, a mobile phone isn't always the best to be efficient. A good old computer is much better.

I have always had trouble finding a good application to read epub books on the PC, something both simple and fully screen reader accessible.

- You can always rename a .epub to .zip and extract HTML files, but it isn't always easy to find in which order you need to read the files, and you can't add bookmarks
- There are a lot of epub reader applications out there like Adobe Digital Edition, Azardi, Callibre; but at best they are only partially accessible, or not very practical to use with a screen reader
- There also are a lot of browser extensions like epubreader for firefox, readium for chrome; but again, accessibility is often not that good, and much worse than when using the browser to navigate on the web.

There come 6reader.

The idea to add scripting capabilities using XQuilla come from Mailtoloco (https://www.stsolution.org/).
as well as Jaws ResearchIt.

# Dependencies
6reader needs the following dependencies in order to be compiled:

- wxWidgets 3.1.3+
- CURL with HTTPS support
- XQuilla

# Download
Latest download can be found here
http://vrac.quentinc.net/6reader.zip
