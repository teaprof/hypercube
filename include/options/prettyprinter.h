#ifndef __PRETTY_PRINTER_H__
#define __PRETTY_PRINTER_H__

#include <sstream>
#include <string>
#include <iostream>
#include <vector>

class TextSection {
    public:
    std::stringstream title;
    std::stringstream header;
    std::stringstream body;
    std::vector<TextSection> subsections;
    std::stringstream footer;

    void print(size_t level) {
        printTitle(level, title);
        if(!title.view().empty())
            level++;
        printText(level, header);
        printText(level, body);
        for(auto& subsection : subsections) {
            subsection.print(level);
        }
        printText(level, footer);
    }
    private:

    static void printTitle(size_t level, const std::stringstream& str) {
        if(str.view().empty()) {
            return;
        }        
        std::cout<<"\n"<<addPrefix(titleprefix(level), str.view())<<std::endl;
    }
    static void printText(size_t level, const std::stringstream& str) {
        if(str.view().empty()) {
            return;
        }        
        std::cout<<addPrefix(textprefix(level), str.view())<<std::endl;
    }
    static std::string addPrefix(const std::string& prefix, const std::string_view& view) {
        if(view.empty()) {
            return "";
        }
        std::stringstream res;
        res<<prefix;
        for(auto ch : view) {
            res<<ch;
            if(ch == '\n') {
                res<<prefix;
            }
        }
        return res.str();
    }
    static std::string titleprefix(size_t level) {
        return prefix(level, "**");
    }
    static std::string textprefix(size_t level) {
        return prefix(level, "  ");
    }
    static std::string prefix(size_t level, const std::string& pattern) {
        std::string res;
        for(size_t n = 0; n < level; n++)
            res += pattern;
        return res;
    }
};

#endif