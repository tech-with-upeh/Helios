#pragma once
//#include <vector>
#include <string>
#include <unordered_map>


//helios specific
//#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"

class PreProcess {
	public:
		PreProcess();
		void processError(const std::string &message, AST_NODE* current);
		PreprocessRet *process(AST_NODE* root, std::string scope = "root");
};
