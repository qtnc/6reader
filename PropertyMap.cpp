#include "PropertyMap.hpp"
#include<boost/algorithm/string.hpp>
#include<fstream>
#include<vector>
using namespace std;
using namespace boost;

bool PropertyMap::load (const string& filename) {
ifstream in(filename);
if (in) return load(in);
else return false;
}

bool PropertyMap::save (const string& filename) {
ofstream out(filename);
if (out) return save(out);
else return false;
}

bool PropertyMap::load (istream& in) {
string line;
while(getline(in, line)) {
trim(line);
if (line.empty() || line[0]=='#' || line[0]==';' || starts_with(line, "//")) continue;
auto n = line.find('=');
if (n<=0 || n==string::npos) continue;
string key = line.substr(0, n), value = line.substr(n+1);
trim(key);
if (flags&PM_LCKEYS) to_lower(key);
if (flags&PM_BKESC) {
replace_all(value, "\\r\\n", "\r\n");
replace_all(value, "\\t", "\t");
}
trim(value);
map[key]=value;
}
return !!in;
}

bool PropertyMap::save (ostream& out) {
vector<string> keys;
for (auto& p: map) keys.push_back(p.first);
sort(keys.begin(), keys.end() );
for (auto& key: keys) {
out << key << '=' << map[key] << endl;
}
return !!out;
}
