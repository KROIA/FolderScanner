#include "stringUtility.h"


string &replaceAll(string &str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
string getReplaceAll(string str, const string& from, const string& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}
string getLower(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}
string getUpper(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}
string &toLower(string &str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}
string &toUpper(string &str)
{
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

bool isNumber(const std::string& s)
{
    return( strspn( s.c_str(), "-.0123456789" ) == s.size() && s.size());
}
string doubleToStr(double d,bool removeCommaIfPossible)
{
    string str = std::to_string(d);

    size_t i;
    for(i=str.size(); i>0; --i)
    {
        if(str[i-1] != '0')
            break;
    }

    if(removeCommaIfPossible)
    {
        str = str.substr(0,i);
        if(str.find(".") != string::npos)
        {
            if(strspn(str.substr(str.find(".")+1).c_str(), "0123456789") == 0)
                str = str.substr(0,str.find("."));
        }
    }
    else
    {
        if(i - str.find(".") > 1 )
            str = str.substr(0,i);
        else
            str = str.substr(0,i+1);
    }


    return str;
}
string intToStr(long int i,unsigned int digits)
{
    std::stringstream ss;
    ss << std::setw(digits) << std::setfill('0') << i;
    std::string s = ss.str();
    return s;
}
string  getTimeString(double seconds)
{
    char str[30];
    string t;
    int ms  = (long long)(seconds*1000)%1000;
    int sec = (long long)seconds%60;
    int min = ((long long)seconds/60)%60;
    int hours = ((long long)seconds/3600);

    if(hours)
    {
        sprintf(str,"%4ih ",hours);
        t+=string(str);
    }
    if(min || hours)
    {
        sprintf(str,"%4imin ",min);
        t+=string(str);
    }
    if(sec || min || hours)
    {
        sprintf(str,"%4is",sec);
        t+=string(str);
    }
    if(ms || sec || min || hours)
    {
        sprintf(str,"%4ims",ms);
        t+=string(str);
    }
    return t;

}

int  getLinePosOf(const string &target,const vector<string> &list)
{
    //internalDebugTabIn();
    //debug("Suche in Liste nach: \""+target+"\"",debugmode::some);
    for(size_t line=0; line<list.size(); line++)
    {
        if(list[line].find(target) != string::npos)
        {
            //debug("Gefunden in Zeile: "+std::to_string(line),debugmode::some);
            //internalDebugTabOut();
            //STATS__searchedStringsInFiles++;
            return line;
        }
    }
    //internalDebugTabOut();
    return -1;
}

vector<string> &append(vector<string> &list, const vector<string> &toAppend)
{
    list.insert(list.begin(),toAppend.begin(),toAppend.end());
    return list;
}

string allignedNewlineText(size_t spaces,string text)
{
    string ret;
    //const std::regex regExp("[ ]+[^ ].*");
    //bool match = std::regex_match(text, regExp);
    //int startIndex = 0;
    string space = "";

    /*if(match)
    {
        std::smatch pieces_match;
        const std::regex token("([ ]+)([^ ].*)");
        std::regex_match(text, pieces_match, token);
        spaces = pieces_match[1];
        startIndex = text.find(pieces_match[2]);
    }*/
    for(size_t i=0; i<spaces; ++i)
        space+=" ";

    if(text.find("\n") == string::npos)
    {
        ret = /*space+*/text;
    }
    else
    {
        ret = /*space+*/text.substr(0,text.find("\n")+1);
        text = text.substr(text.find("\n")+1,text.size());



        while(text.find("\n") != string::npos)
        {
            ret += space + text.substr(0,text.find("\n")+1);
            text = text.substr(text.find("\n")+1,text.size());
        }

        //if(hasNewLines)
            ret += space + text;
    }
    return ret;
}
vector<string>     getEqualSpaced(vector<vector<string>> list)
{
    vector<size_t> columnWidthList;
    return getEqualSpaced(list,columnWidthList);
}

vector<string>     getEqualSpaced(vector<vector<string>> list,vector<size_t> &columnWidthList)
{
    vector<string> retList;
    if(list.size() == 0)
        return retList;
    const size_t columnSpace = 4;
    size_t columns = 0;
    size_t rows    = list.size();

    for(size_t row=0; row<rows; row++)
    {
        if(list[row].size() > columns){columns = list[row].size();}
    }

    columnWidthList.clear();
    columnWidthList.reserve(columns);
    for(size_t column=0; column<columns; column++)
    {
        size_t columnWidth = 0;
        for(size_t row=0; row<rows; row++)
        {
            if(list[row].size() <= column)
                continue; //Skip because empty column in this row
            if(list[row][column].size() > columnWidth){columnWidth = list[row][column].size();}
        }
        columnWidthList.push_back(columnWidth+columnSpace);
    }
    for(size_t row=0; row<rows; row++)
    {
        string line ="";
        for(size_t column=0; column<columns; column++)
        {
            if(list[row].size() <= column)
                continue; //Skip because empty column in this row
            string spaces = "";
            for(size_t space=0; space<columnWidthList[column] - list[row][column].size(); space++)
            {
                spaces+= " ";
            }
            if(column == columns-1)
                line += list[row][column];
            else
                line += list[row][column] +  spaces;
        }

        retList.push_back(line);
    }
    return retList;
}
