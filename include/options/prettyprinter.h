#ifndef __PRETTY_PRINTER_H__
#define __PRETTY_PRINTER_H__

#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <locale>
#include <memory>

class TextSectionBase {
    public:
    virtual std::stringstream title() const = 0;
    virtual std::stringstream header() const = 0;
    virtual std::stringstream body() const = 0;
    virtual std::stringstream footer() const = 0;
    virtual std::vector<std::shared_ptr<TextSectionBase>> subsections() const = 0;
};

class TextSectionAutoBody : public TextSectionBase {
    public:
    std::stringstream title_;
    std::stringstream header_;
    std::stringstream footer_;    
    std::vector<std::shared_ptr<TextSectionBase>> subsections_;
    void setTitle(const std::string& str) {
        title_.clear();
        title_<<str;
    }
    void setHeader(const std::string& str) {
        header_.clear();
        header_<<str;
    }
    void setFooter(const std::string& str) {
        footer_.clear();
        footer_<<str;
    }
    std::stringstream title() const override {
        std::stringstream res;
        res<<title_.str();
        return res;
    }
    std::stringstream header() const override {
        std::stringstream res;
        res<<header_.str();
        return res;
    }
    std::stringstream body() const override = 0;
    std::stringstream footer() const override {
        std::stringstream res;
        res<<footer_.str();
        return res;
    }
    std::vector<std::shared_ptr<TextSectionBase>> subsections() const  override {
        return subsections_;
    }
};

class TextSection : public TextSectionAutoBody {
    public:
    std::stringstream body_;
       
    std::stringstream body() const override {
        std::stringstream res;
        res<<body_.str();
        return res;
    }
};


class PrettyPrinter {
    public:

    static void print(const TextSectionBase& section) {
        print(section, 0);
    }

    static void print(const TextSectionBase& section, size_t level) {
        printTitle(level, section.title());
        /*if(!title.view().empty())
            level++;*/
        printText(level, section.header());
        printText(level, section.body());
        for(auto& subsection : section.subsections()) {
            print(*subsection, level+1);
        }
        printText(level, section.footer());
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