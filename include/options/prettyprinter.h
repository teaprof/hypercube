#ifndef __PRETTY_PRINTER_H__
#define __PRETTY_PRINTER_H__

#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <locale>

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
        std::cout<<"\n"<<print(titleprefix(level), toUpper(str.view()), titlesuffix(level))<<std::endl;
    }
    static void printText(size_t level, const std::stringstream& str) {
        if(str.view().empty()) {
            return;
        }        
        std::cout<<print(textprefix(level), str.view(), textsuffix(level))<<std::endl;
    }
    static std::string print(const std::string& lineprefix, const std::string_view& view, const std::string& linesuffix) {
        if(view.empty()) {
            return "";
        }
        std::stringstream res;
        res<<lineprefix;
        for(auto ch : view) {            
            if(ch == '\n') {
                res<<linesuffix<<"\n";
                res<<lineprefix;
            } else {
                res<<ch;
            }
        }
        res<<linesuffix;
        return res.str();
    }
    static std::string titleprefix(size_t level) { 
        return repeat(level, "  ") + bold();
    }
    static std::string titlesuffix(size_t level) {
        return reset();
    }
    static std::string textprefix(size_t level) {
        return repeat(level, "  ");
    }
    static std::string textsuffix(size_t level) {
        return "";
    }
    static std::string repeat(size_t level, std::string pattern=" ") {
        std::string res;
        for(size_t n = 0; n < level; n++)
            res += pattern;
        return res;
    }
    static std::string toUpper(const std::string_view& str) {
        std::string res;
        for(auto ch : str) {
            res += std::toupper(ch);
        }
        return res;
    }
    //For esc-codes, see https://man7.org/linux/man-pages/man4/console_codes.4.html
    static std::string underline() {
        return "\033[4m";
    }
    static std::string bold() {
        return "\033[1m";
    }
    static std::string red() {
        return "\033[31m";
    }    
    static std::string reset() {
        return "\033[0m";
    }
};

#endif