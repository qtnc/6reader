#include "stringUtils.hpp"
#include<sstream>
#include "cpprintf.hpp"
using namespace std;

string formatTime (int n) {
if (n<3600) return format("%0$2d:%0$2d", n/60, n%60);
else return format("%d:%0$2d:%0$2d", n/3600, (n/60)%60, n%60);
}

string formatSize (long long size) {
static const char suffixes[] = "oKMGT";
const char* suffix = suffixes;
double divider = 1;
while(size/divider>1000) {
suffix++;
divider*=1024;
}
return format("%.3g%c", size/divider, *suffix);
}


string urldecode (const string& u) {
if (string::npos==u.find('%')) return u;
string s;
s.reserve(u.size());
for (size_t i=0, n=u.size(); i<n; i++) {
if (u[i]!='%') s+=u[i];
else {
size_t val='?';
istringstream in(string(&u[i+1], &u[i+3]));
in >> hex >> val;
i+=2;
s+=(char)val;
}}
return s;
}

