#ifndef _____PROPERTY_MAP_____1
#define _____PROPERTY_MAP_____1
#include<string>
#include<unordered_map>
#include<sstream>

#define PM_LCKEYS 1
#define PM_BKESC 2

class PropertyMap {
private:
std::unordered_map<std::string, std::string> map;
int flags;

public:
PropertyMap (int flags = 0): flags(flags) {}
~PropertyMap () = default;

std::unordered_map<std::string, std::string>& kvmap () { return map; }


inline int getFlags () { return flags; }
inline void setFlags (int f) { flags=f; }

template<class T> T get (const std::string& key, const T& def) {
auto it = map.find(key);
if (it==map.end()) return def;
T value;
std::istringstream in(it->second);
in >> std::boolalpha >> value;
return value;
}

template<class T> T get (const std::string& key) {
return get(key, T());
}

template<class T> void set (const std::string& key, const T& value) {
std::ostringstream out;
out << std::boolalpha << value;
map[key] = out.str();
}

inline std::string get (const std::string& key, const std::string& def="") {  
auto it = map.find(key);
return it==map.end()? def : it->second;
}

inline void set (const std::string& key, const std::string& value) { 
map[key]=value; 
}

inline std::string get (const std::string& key, const char* def) {  
auto it = map.find(key);
return it==map.end()? std::string(def) : it->second;
}

inline void set (const std::string& key, const char* value) { 
map[key]=value; 
}

inline bool toggle (const std::string& key, bool def=false) {
def = !get(key, def);
set(key, def);
return def;
}

bool contains (const std::string& key) {
return map.find(key) != map.end();
}

inline bool erase (const std::string& key) {
auto it = map.find(key);
if (it==map.end()) return false;
map.erase(it);
return true;
}

bool load (std::istream& in);
bool load (const std::string& filename);
bool save (std::ostream& out);
bool save (const std::string& filename);
};

#endif
