#pragma once
#include <vector>
#include <string>


//helios specific
#include "lexer.hpp"
#include "parser.hpp"

class PreProcess {
	public:
		PreProcess();
		AST_NODE * process(AST_NODE* root);
};
