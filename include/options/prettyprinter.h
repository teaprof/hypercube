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
    std::stringstream footer;
    std::vector<TextSection> subsections;

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

    static void printTitle(size_t level, const std::stringstream& str) {
        if(str.view().empty()) {
            return;
        }    
        std::cout<<"\n"<<addPrefix(prefix(level), str.view())<<std::endl;
    }
    static void printText(size_t level, const std::stringstream& str) {
        if(str.view().empty()) {
            return;
        }        
        std::cout<<addPrefix(prefix(level), str.view())<<std::endl;
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
    static std::string prefix(size_t level) {
        std::string res;
        for(size_t n = 0; n < level; n++)
            res += "  ";
        return res;
    }
};

#endif