#include "preprocessor.hpp"
#include "astvisualise.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "utils.hpp"
#include "semantics.hpp"
// #include "astvisualise.hpp"

#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>



namespace fs = std::filesystem;

PreProcess::PreProcess() {};

PreprocessRet* PreProcess::process(AST_NODE * root, std::string scope) {
	std::unordered_map<VarScopeInfo, VarInfo> ProcImports;	
	for(int i = 0; i < root->SUB_STATEMENTS.size(); i++) {
		AST_NODE* node = root->SUB_STATEMENTS.at(i);
		if (!node)
		{
			continue;
		}

		if (node->TYPE == NODE_FROM_IMPORT) {
	                std::vector<std::string> objs;	
			for (auto &nobj : node->SUB_STATEMENTS) {
				if(!nobj) {
					continue;
				}
				if (nobj->TYPE == NODE_OBJ) {
					objs.push_back(*(nobj->value));
				}
			}
			node->SUB_STATEMENTS.clear();

			std::string importPath = *(node->value);
			if (!importPath.ends_with(".ink")) {
				importPath = importPath + ".ink";
			}
			if(!fs::exists(importPath)) {
				processError("Import file not found: '" + importPath + "'", node);
			}
			std::ifstream importFile(importPath);
			if (!importFile.is_open()) {
    				processError("Failed to open file: '" + importPath + "'", node);
			}	
			std::stringstream buffer;
			char temp;
			while (importFile.get(temp))
			{
				buffer << temp; 
			}
			buffer << ' '; // EOF marker
			std::string importSource = buffer.str();
			//lex
			Lexer lexer(importSource);  
			std::vector<Token *> tokens = lexer.tokenize();
			//parse
			Parser parser(tokens);
			AST_NODE* importedRoot = parser.parse();
			PreprocessRet *processedImportedRoot = process(importedRoot);
				

			/*
			std::cout << "\n\n-----iiiii-----\n";
			printAST(processedImportedRoot);
			std::cout << "----------------------------------------------------------------------\n";
			*/
			for (auto &stmt : processedImportedRoot->ProcessAstNodes->SUB_STATEMENTS) {
				if (!stmt) {
					continue;
				}
				if (stmt->TYPE == NODE_VARIABLE) {
					if(stmt->CHILD) {
						auto it = std::find(objs.begin(), objs.end(), *(stmt->value));
						if (it != objs.end()) {
							//node->SUB_STATEMENTS.push_back(stmt);
							objs.erase(it);
							VarType importType = SemanticAnalyzer().checkNode(stmt, false, false, false, scope); 
							ProcImports[VarScopeInfo{*(stmt->value), scope}] = {importType, true};  
								//*(stmt->value);
							
						}
					}
					continue;
				}
				std::cout << nodetostr(stmt->TYPE) << "  --ppppp\n";
				node->SUB_STATEMENTS.push_back(stmt);
 			}
			std::stringstream unresolvedImports;	
			if (!objs.empty()) {
				for (auto &str : objs) {
					unresolvedImports << str << ", ";
				}
				processError("Unable to resolve the following Imports: '" + unresolvedImports.str() + "'", node);	
			}
		}
		if(node->TYPE == NODE_IMPORT) {
			std::string importPath = *(node->value);
			if (!importPath.ends_with(".ink")) {
				importPath = importPath + ".ink";
			}
			if(!fs::exists(importPath)) {
				processError("Import file not found: '" + importPath + "'", node);
			}
			std::ifstream importFile(importPath);
			if (!importFile.is_open()) {
    				processError("Failed to open file: '" + importPath + "'", node);
			}	
			std::stringstream buffer;
			char temp;
			while (importFile.get(temp))
			{
				buffer << temp; 
			}
			buffer << ' '; // EOF marker
			std::string importSource = buffer.str();
			Lexer lexer(importSource);  
			std::vector<Token *> tokens = lexer.tokenize();
			Parser parser(tokens);
			AST_NODE* importedRoot = parser.parse();
			PreprocessRet* processedImportedRoot = process(importedRoot);
			for (auto &stmt : processedImportedRoot->ProcessAstNodes->SUB_STATEMENTS) {
				node->SUB_STATEMENTS.push_back(stmt);
			}
		}
	}
	PreprocessRet *RetObj = new PreprocessRet();
	RetObj->ProcessAstNodes = root;
	RetObj->ImportVec = ProcImports;
	return RetObj;
}




void PreProcess::processError(const std::string &message, AST_NODE* current) {
        std::cerr << "\nPreprocessor Error: " << message
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
         throw std::runtime_error("Preprocessor Error"); 
    }
