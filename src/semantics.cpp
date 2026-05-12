#include "semantics.hpp"
#include "parser.hpp"
//#include "preprocessor.hpp"
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
//#include <span>


 std::string  vartypestr(VarType type) {
    switch (type) {
        case TYPE_INT: return "INT"; break;
        case TYPE_STRING: return "STRING"; break;
        case TYPE_BOOL: return "BOOL"; break;
        case TYPE_FUNCTION: return "FUNCTION"; break;
        case TYPE_DICT: return "DICT"; break;
        case TYPE_FLOAT: return "FLOAT"; break;
        default: return "UNKNOWN"; break;
    }
}

SemanticAnalyzer::SemanticAnalyzer() {
     draw_callables = {
        {"clear", {{}, false, true, TYPE_UNKNOWN} },
        {"setFill",     {{ TYPE_STRING }, false, true, TYPE_UNKNOWN}},
        {"setStroke",   {{ TYPE_STRING }, false, true, TYPE_UNKNOWN}},
        {"lineWidth",   {{ TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"alpha",       {{ TYPE_INT }, false, true, TYPE_UNKNOWN}},

        {"rect",        {{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"strokeRect",  {{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"line",        {{ TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},

        {"circle",        {{ TYPE_INT, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"strokeCircle",  {{ TYPE_INT, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},

        {"font",        {{ TYPE_STRING }, false, true, TYPE_UNKNOWN}},
        {"text",        {{ TYPE_STRING, TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},

        {"move",        {{ TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"rotate",      {{ TYPE_INT }, false, true, TYPE_UNKNOWN}},
        {"scale",       {{ TYPE_INT, TYPE_INT }, false, true, TYPE_UNKNOWN}}
    };
    platform_callables = {
        {"height",{{}, true, false, TYPE_FLOAT}},
        {"width",{{}, true, false, TYPE_FLOAT}},
        {"scrollY",{{}, true, false, TYPE_FLOAT}}
    };
}

void SemanticAnalyzer::analyze(AST_NODE *root, std::unordered_map<VarScopeInfo, VarInfo> procImports) {
        scope.clear();
        statevars.clear();
        declaredFunctions.clear();
        calledFunctions.clear();
        pagescope.clear();
        instances.clear();

        
        instances["platform"] =  {platform_callables, true};
        instances["draw"] = {draw_callables, true};

        scope.merge(procImports);

        // Pass 1: Analyze all statements
        for (const auto &stmt : root->SUB_STATEMENTS) {
            checkNode(stmt);
        }

        // Pass 2: Validate functions that were called but not declared
        for (const auto &fname : calledFunctions) {
            if (declaredFunctions.find(fname) == declaredFunctions.end()) {
                semanticError("Function '" + fname + "' called but not declared.");
            }
        }
}



void SemanticAnalyzer::parserError(const std::string &message, AST_NODE* current) {
        std::cerr << "\nSemantics Error: " << message
                  << " at line " << current->lineno
                  << ", column " << current->charno << "\n";
        std::cerr << "  " << current->lineno << " | " << current->sourceLine << "\n";
        for (int i = 1; i < (current->charno+std::to_string(current->lineno).length()+1); ++i)
            std::cerr << " ";
        for (int i = 0; i < current->value->length(); i++)
        {
            std::cerr << "^";
        }
        std::cerr << "\n\n";
         throw std::runtime_error("Semantics Error"); 
    }

VarType SemanticAnalyzer::checkNode(AST_NODE *node, bool uiexceptonstylsheet, bool funcdecl, bool isfrompage, std::string varscope) {
    if (!node) return TYPE_UNKNOWN;

    switch (node->TYPE) {
        case NODE_INT:
            return TYPE_INT;

        case NODE_STRING:
            return TYPE_STRING;

        case NODE_BOOL: {
            VarType var1 = checkNode(node->SUB_STATEMENTS[0]);
            VarType var2 = checkNode(node->SUB_STATEMENTS[1]);
            if (var1 != var2)
            {
                parserError("Variables don't match '" + *(node->SUB_STATEMENTS[0]->value) + "'" + "  &  " + "'" + *(node->SUB_STATEMENTS[1]->value) + "' doesnt match!.", node);
            }

            return TYPE_BOOL;
        }
        case NODE_FLOAT:
            return TYPE_FLOAT;

        case NODE_DICT: {
            for (const auto& i : node->SUB_STATEMENTS)
            {
                if (!uiexceptonstylsheet)
                {
                    if (i->SUB_STATEMENTS[0]->TYPE == NODE_VARIABLE)
                    {
                        if (node->value)
                        {
                            if (*(node->value) !=  std::string("#"))
                            {
                                VarType rhsType = checkNode(i->SUB_STATEMENTS[0]);
                            }
                        }  else {
                                VarType rhsType = checkNode(i->SUB_STATEMENTS[0]);
                        }
                    }
                }

                if (i->SUB_STATEMENTS[1]->TYPE == NODE_VARIABLE)
                {
                    VarType rhsType = checkNode(i->SUB_STATEMENTS[1]);
                }
            }

            return TYPE_DICT;
        }

        // Variable declaration or assignment
        case NODE_SETSTATE:
        case NODE_VARIABLE: {
            std::string name = *node->value;
            if (node->CHILD) {
                VarType rhsType = checkNode(node->CHILD, uiexceptonstylsheet, funcdecl, isfrompage);
                if (node->CHILD->TYPE == NODE_DRAW) {
                    instances[name] = {draw_callables, true};
                }
                if (node->CHILD->TYPE == NODE_PLATFORM_CLS) {
                    instances[name] = {platform_callables, true};
                }
                if(node->TYPE == NODE_VARIABLE) {
                    scope[VarScopeInfo{name, varscope}] = {rhsType, true};
                    if(node->CHILD->TYPE == NODE_LIST) {
                        ListScopes[name] = {TYPE_LIST, node->CHILD->SUB_STATEMENTS.size()};
                    }
                    if(node->CHILD->TYPE == NODE_DICT) {
                        ListScopes[name] = {TYPE_DICT, node->CHILD->SUB_STATEMENTS.size()};
                    }
                }
                if(node->TYPE == NODE_SETSTATE) {
                    statevars[name] = {rhsType, true};
                }
                return rhsType;
            } else {
                // variable usage
                auto it = scope.find(VarScopeInfo{name, varscope});
                std::cout << "usage___> " << varscope << std::endl;
                auto st = statevars.find(name);

                if(st == statevars.end()) {
                    if(it == scope.end()) {
                        parserError("Variable '" + name + "'a used before assignment in scope "+varscope +" .", node);
                    }
                }
                if (st != statevars.end()) {
                    return statevars[name].type;
                }
                if (it != scope.end()) {
                    return scope[VarScopeInfo{name, varscope}].type;
                }

                parserError("Variable '" + name + "'b used before assignment.", node);
                return TYPE_UNKNOWN;

            }
        }
        case NODE_page: {
            if (node->CHILD) {
                VarType node1 = checkNode(node->CHILD->SUB_STATEMENTS[0]);
                if (node1 != TYPE_STRING)
                {
                    parserError("Title can only be a string but got: '" + *(node->CHILD->SUB_STATEMENTS[0]->value) + "'", node->CHILD->SUB_STATEMENTS[1]);
                }

                bool isindex = false;
                if (node->CHILD->SUB_STATEMENTS.size() > 1)
                {
                    for (auto it = node->CHILD->SUB_STATEMENTS.begin() +1; it != node->CHILD->SUB_STATEMENTS.end(); ++it)
                    {
                        AST_NODE* it_node = *it;
                        if(*(it_node->value) == "route") {
                            if (*(it_node->CHILD->value) == "/")
                            {
                                isindex = true;
                            }
                            else
                            {
                                isindex = false;
                            }
                            auto checkroute = pagescope.find(*(it_node->CHILD->value));
                            if (checkroute == pagescope.end()) {
                                pagescope[*(it_node->CHILD->value)] = {*(node->CHILD->SUB_STATEMENTS[0]->value), isindex};
                            } else {
                                parserError("Route '" + *(it_node->CHILD->value) + "' already: '" + checkroute->second.title + "'", node->CHILD->SUB_STATEMENTS[0]);
                            }

                        }
                        VarType node2 = checkNode(it_node->CHILD, true);
                        if (node2 != TYPE_DICT && node2 != TYPE_STRING)
                        {
                            parserError("Unknown Type in Args in page() but got: '" + *(it_node->value) + "'", it_node);
                        }
                    }
                    if (isindex) {
                        auto pagesc = pagescope.find("/");
                        if (pagesc == pagescope.end()) {
                            pagescope["/"] = {*(node->CHILD->SUB_STATEMENTS[0]->value), true};
                        } else {
                            parserError("Index page already defined: '" + pagesc->second.title + "'", node->CHILD->SUB_STATEMENTS[0]);
                        }
                    }

                } else {
                    auto pagesc = pagescope.find("/");

                    if (pagesc == pagescope.end()) {
                        pagescope["/"] = {*(node->CHILD->SUB_STATEMENTS[0]->value), true};
                    } else {
                        parserError("Index page already defined as: '" + pagesc->second.title + "'", node->CHILD->SUB_STATEMENTS[0]);
                    }
                }
            }
            if (!node->SUB_STATEMENTS.empty())
            {
                for (auto &i : node->SUB_STATEMENTS)
                {
                    if (i->TYPE == NODE_FUNCTION_DECL && *(i->value) == "onmount")  {
                        checkNode(i, false, true, true);
                        continue;
                    }
                    checkNode(i, true,false, true);
                }
            }
            statevars.clear();
            instances.clear();
            instances["platform"] =  {platform_callables, true};
            instances["draw"] = {draw_callables, true};
            return TYPE_FUNCTION;
        }

        case HELIOS_NODE_TEXT:
        case NODE_CANVAS:
        case NODE_IMAGE:
        case NODE_INPUT:
        case NODE_VIEW: {
            if (node->CHILD) {

                VarType node1 = checkNode(node->CHILD->SUB_STATEMENTS[0]);
                if (node1 != TYPE_STRING)
                {
                    parserError("Title can only be a string but got: '" + *(node->CHILD->SUB_STATEMENTS[0]->value) + "'", node->CHILD->SUB_STATEMENTS[0]);
                }

                if (node->CHILD->SUB_STATEMENTS.size() > 1)
                {
                    for (auto it = node->CHILD->SUB_STATEMENTS.begin() +1; it != node->CHILD->SUB_STATEMENTS.end(); ++it)
                    {
                        AST_NODE* it_node = *it;
                        VarType node2 = checkNode(it_node, true, true);
                        if (node2 != TYPE_DICT && node2 != TYPE_STRING && node2 != TYPE_FUNCTION && node2 != TYPE_INT && node2 != TYPE_FLOAT && node2 != TYPE_BOOL)
                        {
                            parserError("Unknown Type in Args in View() but got: '" + *(it_node->value) + "<->" + *(it_node->CHILD->value) + "'", it_node->CHILD);
                        }
                    }
                }
            }
            if (!node->SUB_STATEMENTS.empty())
            {
                for (auto &i : node->SUB_STATEMENTS)
                {
                    checkNode(i);
                }
            }
            return TYPE_FUNCTION;
        }
        // Binary operations (e.g., +, -, ==)
        case NODE_BINARY_OP:
        case NODE_COMPARISON_OP: {
            VarType leftType = checkNode(node->SUB_STATEMENTS[0], false, false, false, varscope);
            VarType rightType = checkNode(node->SUB_STATEMENTS[1]);
            std::string op = *node->value;

            if (leftType == TYPE_INT ||  leftType == TYPE_FLOAT) {
                if (rightType == TYPE_INT || rightType == TYPE_FLOAT) {
                    return TYPE_FLOAT;
                }
            }
            if (leftType != rightType) {
                parserError("Type mismatch in binary operation, can't "+ vartypestr(leftType) + " " + op + " " + vartypestr(rightType), node);
            }

            if (op == "*" || op == "/") {
                if (leftType != TYPE_INT && leftType != TYPE_FLOAT)
                    parserError("Operator '" + op + "' only supports numbers.", node);
                return TYPE_INT;
            }

            if (op == "==" || op == "!=" || op == "<" || op == ">")
                return TYPE_BOOL;

            return TYPE_UNKNOWN;
        }

        // Function declaration
        case NODE_FUNCTION_DECL: {
            std::string name = *node->value;
            declaredFunctions[name] = {TYPE_FUNCTION, true};

            // Register arguments in scope
            if (node->CHILD && node->CHILD->TYPE == NODE_ARGS) {
                for (auto param : node->CHILD->SUB_STATEMENTS) {
                    if (funcdecl)
                    {
                        std::string name = *param->value;

                        auto it = scope.find(VarScopeInfo{name, varscope});
                        scope.find(VarScopeInfo{name, varscope});
                        auto st = statevars.find(name);

                            if(st == statevars.end()) {
                                if(it == scope.end()) {
                                    parserError("Variable '" + name + "'c used before assignment.", node);
                                }
                            }
                        if (it != scope.end()) {
                                return scope[VarScopeInfo{name, varscope}].type;
                        }
                        if (st != statevars.end()) {
                                return statevars[name].type;
                        }
                    }
                    else
                    {
                        if (param->TYPE == NODE_VARIABLE) {
                            std::string paramName = *param->value;
                            scope[VarScopeInfo{paramName, varscope}] = {TYPE_UNKNOWN, true};
                    }
                    }
                }
            }

            // Check the function body
            for (auto stmt : node->SUB_STATEMENTS) {
                checkNode(stmt);
            }
            return TYPE_FUNCTION;
        }

        // Function call (even undeclared — checked in post-pass)
        case NODE_FUNCTION_CALL: {
            std::string fname = *node->value;
            calledFunctions.push_back(fname);

            // Evaluate arguments for validity
            for (auto arg : node->SUB_STATEMENTS)
                checkNode(arg);

            if (declaredFunctions.find(fname) != declaredFunctions.end())
                return declaredFunctions[fname].type;

            return TYPE_UNKNOWN;
        }

        // Return and print
        case NODE_TOINT:
        case NODE_TOSTR:
        case NODE_TOFLOAT:
        case NODE_RETURN:
        case NODE_PRINT:
        case NODE_GO:
        case NODE_PLATFORM_CLS:
        case NODE_DRAW:
        case NODE_TYPE_CHECK:
            if (node->CHILD) {
                if (node->TYPE == NODE_DRAW) {
                    if (!isfrompage) {
                        //logic to add to instance
                    }
                    VarType ty = checkNode(node->CHILD->SUB_STATEMENTS[0]);
                    if (ty != TYPE_STRING) {
                        parserError("Type Conversion Must be Str.", node->CHILD);
                    }
                    return TYPE_FUNCTION;
                }
                VarType ty = checkNode(node->CHILD, uiexceptonstylsheet, funcdecl, isfrompage, varscope);

                if (node->CHILD->TYPE == NODE_INSTANCE) {
                    return ty;
                }

                if(node->TYPE == NODE_TOFLOAT || node->TYPE == NODE_TOINT) {
                    if (ty != TYPE_STRING) {
                        parserError("Type Conversion Must be Str.", node->CHILD);
                    }
                    if (node->TYPE == NODE_TOINT) {
                        return TYPE_INT;
                    }
                    if (node->TYPE == NODE_TOFLOAT) {
                        return TYPE_FLOAT;
                    }
                }
                if (node->TYPE == NODE_TOSTR) {
                    return TYPE_STRING;
                }
            }

            return TYPE_FUNCTION;



        // If / While / For blocks
        case NODE_IF: {
            if (node->CHILD) {
                VarType condType = checkNode(node->CHILD);
                if (condType != TYPE_BOOL) {
                    parserError("Condition in if statement must evaluate to a boolean.", node->CHILD);
                }
                for (auto ifs : node->CHILD->SUB_STATEMENTS)
                {
                    checkNode(ifs);
                }
                for (auto stmt : node->SUB_STATEMENTS) {
                    checkNode(stmt);
                }
                return TYPE_UNKNOWN;
            }
        }
        case NODE_WHILE: {
            if (node->CHILD) {
                VarType condType = checkNode(node->CHILD);
                if (condType != TYPE_BOOL)
                    semanticError("Condition in if/while must evaluate to a boolean.");
            }
            for (auto stmt : node->SUB_STATEMENTS)
                checkNode(stmt);
            return TYPE_UNKNOWN;
        }
        case NODE_ELSE_IF: {
            if (node->CHILD) {
                VarType condType = checkNode(node->CHILD);
                if (condType != TYPE_BOOL)
                    parserError("Condition in 'else If' must evaluate to a boolean.", node->CHILD);
            }
            for (auto *stmt : node->SUB_STATEMENTS) {
                checkNode(stmt);
            }
            return TYPE_UNKNOWN;
        }

        case NODE_ELSE: {
            for (auto &i : node->SUB_STATEMENTS)
            {
                checkNode(i);
            }
            return TYPE_UNKNOWN;
        }
        case NODE_FOR:
            checkNode(node->CHILD->SUB_STATEMENTS[0]);
            checkNode(node->CHILD->SUB_STATEMENTS[1]);
            checkNode(node->CHILD->SUB_STATEMENTS[2]);
            for (auto stmt : node->SUB_STATEMENTS) {
                checkNode(stmt);
            }
            scope.erase(VarScopeInfo{*(node->CHILD->SUB_STATEMENTS[0]->value), varscope});;
            return TYPE_UNKNOWN;

         case NODE_ADDSTYLE: {
            if (node->CHILD) {
                std::string id = *(node->CHILD->value);
                
                if(std::find(stylesheet_imports.begin(), stylesheet_imports.end(), id) == stylesheet_imports.end()) {
                    parserError("Stylesheet '" + id + "' is not Defined, can't add style to it.", node->CHILD);
                }
            }
            return TYPE_FUNCTION;
        }
        case NODE_REMOVESTYLE: {
            if (node->CHILD) {
                std::string id = *(node->CHILD->value);
                
                if(std::find(stylesheet_imports.begin(), stylesheet_imports.end(), id) == stylesheet_imports.end()) {
                    parserError("Stylesheet '" + id + "' is not Defined, can't remove style to it.", node->CHILD);
                }
            }
            return TYPE_FUNCTION;
         }
         
        case NODE_STYLESHEET:
                if (node->value) {
                    stylesheet_imports.push_back(*(node->value));
                }
            for (auto stmt : node->SUB_STATEMENTS)
                checkNode(stmt, false);
            return TYPE_UNKNOWN;
        case NODE_CLS:
            checkNode(node->CHILD, true);
            return TYPE_UNKNOWN;
        case NODE_MEDIA_QUERY: {
            checkNode(node->CHILD);
            for (auto stmt : node->SUB_STATEMENTS)
                checkNode(stmt, false);
            return TYPE_UNKNOWN;
        }
        case NODE_SCOPE_INSTANCE: {
            if(!node->CHILD) {
                parserError("Scope Instance '" + *(node->value) + "' is missing a child scope.", node);
            }
            if(node->CHILD->TYPE == NODE_SCOPE_INSTANCE) {
                return checkNode(node->CHILD, false, false,false, *(node->value));
            }
            
            if (node->CHILD->TYPE == NODE_VARIABLE) {
                auto it = scope.find(VarScopeInfo{*(node->CHILD->value), *(node->value)});
                if (it == scope.end()) {
                    parserError("Scope '"+ *(node->value) +"' has no child called '"+ *(node->CHILD->value) +"'", node);
                } 
            } else {
                if (node->CHILD->TYPE == NODE_UNARY_OP) {
                    auto it = scope.find(VarScopeInfo{*(node->CHILD->CHILD->value), *(node->value)});
                    if (it == scope.end()) {
                        parserError("Scope '"+ *(node->value) +"' has no child called '"+ *(node->CHILD->CHILD->value) +"'", node);
                    }  
                } else {
                    auto it = scope.find(VarScopeInfo{*(node->CHILD->SUB_STATEMENTS[0]->value), *(node->value)});
                    if (it == scope.end()) {
                        parserError("Scope '"+ *(node->value) +"' has no child called '"+ *(node->CHILD->SUB_STATEMENTS[0]->value) +"'", node);
                    } 
                } 
            }
            std::cout << "Node:--->>>" << *(node->value) << std::endl;
            return checkNode(node->CHILD, false, false, false, *(node->value));
        }
        case NODE_INSTANCE: {
            auto it = instances.find(*(node->value));
            if (it == instances.end()) {
                parserError("'"+ *(node->value) +"'a is not Callable", node);
            }
            if (node->CHILD) {
                auto& secondinstance = it->second;
                auto& calls = secondinstance.callables;

                if(node->CHILD->TYPE == NODE_BINARY_OP) {
                    auto ch = calls.find(*(node->CHILD->SUB_STATEMENTS[0]->value));
                    if (ch == calls.end()) {
                        parserError("'"+ *(node->value) +"' Has no member '"+ *(node->CHILD->SUB_STATEMENTS[0]->value) +"'", node->CHILD);
                    }


                    if (ch->second.isVariadic) {
                            if(node->CHILD->SUB_STATEMENTS[0]->TYPE != NODE_FUNCTION_CALL) {
                                parserError("'" + *(node->CHILD->SUB_STATEMENTS[0]->value) + "' is callable!", node->CHILD);
                            }
                    } else {
                        if(node->CHILD->SUB_STATEMENTS[0]->TYPE == NODE_FUNCTION_CALL) {
                            parserError("'" + *(node->CHILD->SUB_STATEMENTS[0]->value) + "'b is Not callable!", node->CHILD);

                        }
                    }
                    // ch.second.returnType
                    VarType cls = ch->second.returnType;
                    if(cls == TYPE_STRING && node->CHILD->SUB_STATEMENTS[1]->TYPE != NODE_STRING) {
                        parserError("Type Mismatch, can't perfom binary Operation on given types", node->CHILD);
                    }

                    if(cls == TYPE_INT) {
                        if(node->CHILD->SUB_STATEMENTS[1]->TYPE != NODE_FLOAT) {
                            if(node->CHILD->SUB_STATEMENTS[1]->TYPE != NODE_INT) {
                                parserError("Type Mismatch, can't perfom binary Operation on given types", node->CHILD);
                            }
                        }
                    }
                    //if i get lost
                    if(cls == TYPE_STRING) {
                        return TYPE_STRING;
                    }
                    return TYPE_FLOAT;
                }
                auto ch = calls.find(*(node->CHILD->value));
                if (ch == calls.end()) {
                    parserError("'"+ *(node->value) +"' Has no member '"+ *(node->CHILD->value) +"'", node->CHILD);
                }
                if(node->CHILD->SUB_STATEMENTS.empty()) {
                    if (ch->second.isVariadic) {
                        if(node->CHILD->TYPE != NODE_FUNCTION_CALL) {
                            parserError("'" + *(node->CHILD->value) + "' is callable!", node->CHILD);
                        }
                    } else {
                        if (secondinstance.issystemdefined) {
                            if (node->CHILD->CHILD)
                            {
                                parserError("'" + *(node->CHILD->value) + "' is not assignable", node->CHILD);
                            }

                        }
                        if(node->CHILD->TYPE == NODE_FUNCTION_CALL) {
                            parserError("'" + *(node->CHILD->value) + "'c is not Callable", node->CHILD);
                        }
                    }
                } else {
                    if (node->CHILD->SUB_STATEMENTS.size() != ch->second.args.size()) {
                        parserError("'"+ *(node->CHILD->value) +"' was expecting '"+ std::to_string(ch->second.args.size()) +" arguments but got: " + std::to_string(node->CHILD->SUB_STATEMENTS.size()), node->CHILD);
                    }
                    if (ch->second.isVariadic) {
                        if(node->CHILD->TYPE != NODE_FUNCTION_CALL) {
                            parserError("'" + *(node->CHILD->value) + "'d is not callable!", node->CHILD);
                        }
                    } else {
                        if (secondinstance.issystemdefined) {
                            if (node->CHILD->CHILD)
                            {
                                parserError("'" + *(node->CHILD->value) + "' is not assignable", node->CHILD);
                            }

                        }
                    }

                    for (auto &subs : node->CHILD->SUB_STATEMENTS) {
                        for (auto &chv : ch->second.args) {
                            VarType nodecheck = checkNode(subs, uiexceptonstylsheet, funcdecl, isfrompage);
                            if (nodecheck != chv ) {
                                if (nodecheck == TYPE_INT && chv == TYPE_FLOAT) {
                                    continue;
                                } else if (nodecheck == TYPE_FLOAT && chv == TYPE_INT) {
                                    continue;
                                }
                                parserError("'"+ *(node->CHILD->value) +"' was expecting '"+ vartypestr(chv) +"  but got: " + vartypestr(nodecheck), subs);
                            }
                        }
                    }
                }
                return ch->second.returnType;
            }
            
            return TYPE_FUNCTION;
        }
        case NODE_MATH_POW: {
            if (node->CHILD)
            {
                VarType arg = checkNode(node->CHILD, uiexceptonstylsheet, funcdecl,isfrompage);
                if (arg != TYPE_INT && arg != TYPE_FLOAT) {
                    parserError("power only accepts number but got "+ nodetostr(node->CHILD->TYPE), node->CHILD);
                }
                return TYPE_FLOAT;
            }
            for (auto &i : node->SUB_STATEMENTS)
            {
                VarType arg = checkNode(i, uiexceptonstylsheet, funcdecl,isfrompage);
                if (arg != TYPE_INT && arg != TYPE_FLOAT) {
                    parserError("power only--accepts number but got "+ nodetostr(i->TYPE), i);
                }
            }
            return TYPE_FLOAT;
        }
        case NODE_MATH_COS:
        case NODE_MATH_SQRT:
        case NODE_MATH_TAN:
        case NODE_MATH_SIN: {
            VarType arg = checkNode(node->CHILD, uiexceptonstylsheet, funcdecl,isfrompage);
            if (arg != TYPE_INT && arg != TYPE_FLOAT) {
                parserError( nodetostr(node->TYPE) +" only accepts number but got "+ nodetostr(node->CHILD->TYPE), node->CHILD);
            }
            return TYPE_FLOAT;
        }
        case NODE_LIST: {
            if (node->SUB_STATEMENTS.empty()) {
                return TYPE_LIST;
            }
            for (auto &i : node->SUB_STATEMENTS) {
                checkNode(i, uiexceptonstylsheet, funcdecl,isfrompage);
            }
            return TYPE_LIST;
        } case NODE_ID_ATTR: {
            std::string id = *(node->value);
            auto it = scope.find(VarScopeInfo{id, varscope});
            // if(std::find(scope.begin(), scope.end(), id) == scope.end()) {
            //     parserError("Variable '" + id + "' is not Defined, can't use len attribute.", node);
            // }
            if(it == scope.end()) {
                parserError("Variable '" + id + "' is not Defined, can't use len attribute.", node);
            }
            return TYPE_INT;
        } case NODE_INDEXING: {
            // var[index]
            std::string varName = *(node->value);
            auto it = ListScopes.find(varName);
            if (it == ListScopes.end()) {
                parserError("Variable '" + varName + "' is not defined as a list, dict, or string.", node);
            }
            int list_size = static_cast<int>(it->second.tsize);
            if (it->second.type == TYPE_LIST)
            {
                VarType indexType = checkNode(node->CHILD, uiexceptonstylsheet, funcdecl,isfrompage);
                if (indexType != TYPE_INT) {
                    parserError("List indices must be integers but got "+ nodetostr(node->CHILD->TYPE), node->CHILD);
                }
                
                if (node->CHILD->TYPE == NODE_INT) {
                    int indexValue = std::stoi(*(node->CHILD->value));
                    if (indexValue < 0 || indexValue >= list_size) {
                        parserError("List index out of bounds: " + std::to_string(indexValue) + " for list of size " + std::to_string(list_size), node->CHILD);
                    }
                }
            }
            
            return TYPE_ALL; // could be any type depending on the contents of the list/dict/string
        }
        case NODE_IMPORT: {
            std::string importPath = *(node->value);
            for(auto &stmt : node->SUB_STATEMENTS) {
                checkNode(stmt, false, false, false, importPath);
            }
            return TYPE_UNKNOWN;
        }
        default:
            return TYPE_UNKNOWN;
    }
}

void SemanticAnalyzer::semanticError(const std::string &msg) {
    std::cerr << "\n[SemanticError] " << msg << std::endl;
     throw std::runtime_error("SemanticError"); 
}

