#include "preprocessor.hpp"
#include "parser.hpp"
#include "lexer.hpp"

PreProcess::PreProcess();

AST_NODE* PreProcess::process(AST_NODE * root) {
	AST_NODE* NewRootNode = new AST_NODE();
	for(auto node : root->SUB_STATEMENTS) {
		if(node->TYPE == NODE_IMPORT) {
			std::cout << "OKKKK, captured an import\n"
		}
	}
	return NewRootNode;
}
