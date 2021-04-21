#include<iostream>
#include<fstream>
using namespace std;

ofstream debug;

void __attribute__((constructor)) initDebugRedirect (void) {
debug.open("debug.txt");
if (debug) {
cout.rdbuf(debug.rdbuf());
cerr.rdbuf(debug.rdbuf());
}}
