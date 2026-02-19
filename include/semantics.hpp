#pragma once
#include "parser.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

enum VarType {
    TYPE_UNKNOWN,
    TYPE_INT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_FUNCTION,
    TYPE_FLOAT,
    TYPE_DICT,
    TYPE_LIST,
    TYPE_ALL // represents a variable that can be of any type (used for lists/dicts/strings where we don't know the type of the contents)
};

std::string vartypestr(VarType type);

struct VarInfo {
    VarType type;
    bool initialized;
};

struct ListorDictInfo {
    VarType type;
    unsigned long long tsize; 
};

struct PageInfo {
    std::string title;
    bool index;
};

struct CallableInfo {
    std::vector<VarType> args;
    bool hasReturn;
    bool isVariadic;
    VarType returnType;
};

struct InstanceInfo {
    std::unordered_map<std::string, CallableInfo> callables; //
    bool issystemdefined;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    void analyze(AST_NODE *root);

private:
    std::unordered_map<std::string, VarInfo> scope;
    std::unordered_map<std::string, VarInfo> statevars;
    std::unordered_map<std::string, VarInfo> declaredFunctions;
    std::vector<std::string> calledFunctions;
    std::vector<std::string> stylesheet_imports;
    std::unordered_map<std::string, PageInfo> pagescope;
    std::unordered_map<std::string, InstanceInfo> instances;
    std::unordered_map<std::string, CallableInfo> draw_callables;

    std::unordered_map<std::string, CallableInfo> platform_callables;


    std::unordered_map<std::string, ListorDictInfo> ListScopes;

    void parserError(const std::string &message, AST_NODE* current);
    VarType checkNode(AST_NODE *node, bool uiexceptonstylsheet = false, bool funcdecl = false, bool isfrompage = false);
    void semanticError(const std::string &msg);
};
