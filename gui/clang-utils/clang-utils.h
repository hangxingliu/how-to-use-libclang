#include <iostream>
#include <vector>
#include <string>

#include <clang-c/Index.h>

#ifndef LIBCLANG_HEADER
#define LIBCLANG_HEADER

class MyASTNode {
public:
	unsigned id = 0;
	/// 0 means root node(file level)
	int depth = 0;
	unsigned begin = 0;
	unsigned end = 0;

	std::string type;
	std::string name;
	std::vector<MyASTNode*> children;

	MyASTNode() {}
	MyASTNode(unsigned id, std::string type, std::string name, int depth = 0,
		unsigned begin = 0, unsigned end = 0):
		id(id), depth(depth), begin(begin), end(end),
		type(std::move(type)),
		name(std::move(name)) {}

	void addChild(MyASTNode* child) { children.push_back(child); }
};

class MyTokenizeNode {
public:
	unsigned begin = 0;
	unsigned end = 0;
	unsigned line0, line1, col0, col1;

	const char* typeName;
	CXTokenKind type;
	std::string name;

	MyTokenizeNode(const CXTranslationUnit &tu, const CXToken & token);
};

class MyClangParser {
	CXTranslationUnit tu = nullptr;
	CXIndex index = nullptr;

	MyASTNode* rootNode = nullptr;
	std::vector<MyASTNode*> nodesPool;

	std::vector<MyTokenizeNode> nodesToken;

	std::string filePath;
	std::string fileName;

	void dispose() {
		for(MyASTNode* ptr: nodesPool)
			delete ptr;
		nodesPool.clear();
		rootNode = nullptr;
	}

	void copiedWarn() {
		std::cerr << "A MyClangParser be copied! (" << filePath << ")\n"; }

	/// define in libclang.cpp
	void parse();

public:
	MyClangParser(std::string filePath, std::string fileName):
		filePath(std::move(filePath)), fileName(std::move(fileName)) {
		parse(); }
	MyClangParser(const MyClangParser& copyFrom):
		MyClangParser(copyFrom.filePath, copyFrom.fileName){
		copiedWarn(); }
	MyClangParser& operator=(const MyClangParser& copyFrom) {
		this->filePath = copyFrom.filePath;
		this->fileName = copyFrom.fileName;
		this->parse();
		copiedWarn();
		return *this;
	}
	~MyClangParser() { dispose(); }

	/// define in libclang.cpp
	std::vector<std::string> getDiagnosis();

	bool isSuccess() const { return rootNode != nullptr; }

	MyASTNode* getASTRootNode() const {
		return rootNode; }
	const std::vector<MyASTNode*>& getNodesVector() const {
		return nodesPool; }

	MyASTNode* allocateASTNode(
		std::string type, std::string name,
		unsigned begin, unsigned end,
		int depth = 0) {

		auto nodePtr = new MyASTNode(
			(unsigned) nodesPool.size(),
			std::move(type), std::move(name), depth,
			begin, end);
		nodesPool.push_back(nodePtr);
		return nodePtr;
	}

	const std::vector<MyTokenizeNode>& getTokenNodes() const {
		return nodesToken; }
};


#endif // LIBCLANG_HEADER
