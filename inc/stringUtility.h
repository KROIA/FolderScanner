#ifndef STRINGUTILITY_H
#define STRINGUTILITY_H

#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <sstream>
#include <iomanip>

using std::string;
using std::vector;

struct TimeString
{
   /* TimeString(const TimeString& other)
    {

    }*/
    string readableStr;
    string filenameStr;
};



string &replaceAll(string &str, const string& from, const string& to);
string getReplaceAll(string str, const string& from, const string& to);
string getLower(string str);
string getUpper(string str);
string &toLower(string &str);
string &toUpper(string &str);
bool isNumber(const std::string& s);
string doubleToStr(double d, bool removeCommaIfPossible = false);
string intToStr(long int i, unsigned int digits = 0);

string  getTimeString(double seconds);
/*
    Returns     the line of "list" in which "target" was fount the first time.
                Search from list beginning at index 0, up to list.size()-1.
    Returns -1  if "target" couldn't be found in "list".
 */
int    getLinePosOf(const string &target,const vector<string> &list);

vector<string> &append(vector<string> &list, const vector<string> &toAppend);

string allignedNewlineText(size_t spaces,string text);

vector<string>     getEqualSpaced(vector<vector<string>> list, vector<size_t> &columnWidthList);
vector<string>     getEqualSpaced(vector<vector<string>> list);


#endif // STRINGUTILITY_H
