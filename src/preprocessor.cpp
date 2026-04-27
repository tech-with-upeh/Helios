#include "preprocessor.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "astvisualise.hpp"

#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>



namespace fs = std::filesystem;
PreProcess::PreProcess() {};

AST_NODE* PreProcess::process(AST_NODE * root) {
	for(int i = 0; i < root->SUB_STATEMENTS.size(); i++) {
		AST_NODE* node = root->SUB_STATEMENTS.at(i);
		if(node->TYPE == NODE_IMPORT) {
			std::string importPath = *(node->value);
			if (!importPath.ends_with(".ink")) {
				importPath = importPath + ".ink";
			}
			if(!fs::exists(importPath)) {
				processError("Import file not found: '" + importPath + "'", node);
			}

			std::ifstream importFile(importPath);
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

			AST_NODE* processedImportedRoot = process(importedRoot);


			for (auto& stmt : processedImportedRoot->SUB_STATEMENTS) {
				node->SUB_STATEMENTS.push_back(stmt);
			}
		}
	}
	
	return root;
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
