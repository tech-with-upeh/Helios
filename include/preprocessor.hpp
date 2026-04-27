#pragma once
#include <vector>
#include <string>


//helios specific
#include "lexer.hpp"
#include "parser.hpp"

class PreProcess {
	public:
		PreProcess();
		void processError(const std::string &message, AST_NODE* current);
		AST_NODE * process(AST_NODE* root);
};
