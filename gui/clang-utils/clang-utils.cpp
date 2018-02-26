#include <string>
#include <sstream>
#include <vector>

#include <QFile> // for getFileName
#include <QFileInfo>

#include "./clang-utils.h"

using namespace std;

/// from clang string to c++ std::string and dispose clang string;
string convertStr(CXString&& clangStr) {
	string result(clang_getCString(clangStr));
	clang_disposeString(clangStr);
	return result;
}

// ========================================
//
//   MyTokenizeNode:
//
// >>>>>>>>>>>>>>>>>>>

MyTokenizeNode::MyTokenizeNode(const CXTranslationUnit &tu, const CXToken &token) {
	CXTokenKind kind = clang_getTokenKind(token);
	CXSourceRange range = clang_getTokenExtent(tu, token);

	CXSourceLocation beginLoc = clang_getRangeStart(range);
	CXSourceLocation endLoc = clang_getRangeEnd(range);

	CXFile file;
	clang_getExpansionLocation(beginLoc, &file, &line0, &col0, &begin);
	clang_getExpansionLocation(endLoc, &file, &line1, &col1, &end);

	this->name = convertStr(clang_getTokenSpelling(tu, token));
	this->type = kind;
	switch (kind) {
		case CXToken_Punctuation: this->typeName = "Punctuation"; break;
		case CXToken_Keyword:     this->typeName = "Keyword"; break;
		case CXToken_Identifier:  this->typeName = "Identifier"; break;
		case CXToken_Literal:     this->typeName = "Literal"; break;
		case CXToken_Comment:     this->typeName = "Comment"; break;
		default:                  this->typeName = "Unknown"; break;
	}
}



// ========================================
//
//   MyClangParser:
//
// >>>>>>>>>>>>>>>>>>>

class VisitorPackage {
public:
	int depth = 1;
	char _pad[4] = {0};
	MyClangParser* context = nullptr;
	MyASTNode* parent = nullptr;

	VisitorPackage(int depth, MyClangParser* context, MyASTNode* parent):
		depth(depth), context(context), parent(parent) {}

	VisitorPackage goDeeper(MyASTNode* newParent) const {
		return VisitorPackage(depth + 1, context, newParent); }
};

CXChildVisitResult visitor(CXCursor cursor, CXCursor, CXClientData _package) {
	auto pkg = (VisitorPackage*) _package;

	CXSourceLocation location = clang_getCursorLocation(cursor);
	// exclude children not located in main file
	if(!clang_Location_isFromMainFile(location))
		return CXChildVisit_Continue;

	auto range = clang_getCursorExtent(cursor);
	auto cursorKind = clang_getCursorKind(cursor);
	auto name = convertStr(clang_getCursorSpelling(cursor));
	auto kindName = convertStr(clang_getCursorKindSpelling(cursorKind));

	//printCXCursorToken(package->unit, cursor);
	auto beginLoc = clang_getRangeStart(range);
	auto endLoc = clang_getRangeEnd(range);

	CXFile file; unsigned line, col, offset;
	clang_getExpansionLocation(beginLoc, &file, &line, &col, &offset); auto begin = offset;
	clang_getExpansionLocation(endLoc, &file, &line, &col, &offset); auto end = offset;

	auto child = pkg->context->allocateASTNode(
		move(kindName), move(name), begin, end, pkg->depth);
	pkg->parent->addChild(child);

	auto deeperPackage = pkg->goDeeper(child);
	clang_visitChildren(cursor, visitor, &deeperPackage);

	return CXChildVisit_Continue;
}

unsigned getFileSize(const char *filePath) {
	FILE *fp = fopen(filePath, "r");
	fseek(fp, 0, SEEK_END);
	unsigned size = ftell(fp);
	fclose(fp);
	return size;
}

void MyClangParser::parse() {
	const int clangArgc = 1;
	const char* clangArgs[clangArgc];
	clangArgs[0] = "-std=c++11";

	this->index = clang_createIndex(1, 1);
	this->tu = clang_parseTranslationUnit(
		this->index, //CXIndex
		filePath.c_str(),  //source_filename
		clangArgs, clangArgc, // Command line arguments, length
		nullptr, 0, // CXUnsavedFile files, length
		0); // options

	if(this->tu == nullptr)
		return;

	this->rootNode = this->allocateASTNode("Root (File)", this->fileName, 0, 0, 0);
	VisitorPackage package(1, this, this->rootNode);

	auto cursor = clang_getTranslationUnitCursor(this->tu);
	clang_visitChildren(cursor, visitor, &package);

	// generate nodes
	CXFile file = clang_getFile(tu, filePath.c_str());
	unsigned fileSize = getFileSize(filePath.c_str());
	auto begin  = clang_getLocationForOffset(tu, file, 0);
	auto end = clang_getLocationForOffset(tu, file, fileSize);
	auto nil = clang_getNullLocation();
	if(clang_equalLocations(begin, nil) || clang_equalLocations(end, nil))
		return; /// @todo exception handler

	auto range = clang_getRange(begin, end);
	if(clang_Range_isNull(range))
		return; /// @todo exception handler

	CXToken* tokens;
	unsigned tokenCount;
	clang_tokenize(tu, range, &tokens, &tokenCount);
	for(unsigned i = 0 ; i < tokenCount ; i ++ )
		this->nodesToken.emplace_back(MyTokenizeNode(tu, tokens[i]));
	clang_disposeTokens(tu, tokens, tokenCount);

}


string getFileName(string filePath) {
	QFile file(filePath.c_str());
	QFileInfo fi(file);
	return fi.fileName().toStdString();
}

vector<string> MyClangParser::getDiagnosis() {
	vector<string> result;

	CXDiagnosticSet diagSet = clang_getDiagnosticSetFromTU(this->tu);
	unsigned count = clang_getNumDiagnosticsInSet(diagSet);
	for (unsigned i = 0; i < count; i++) {
		CXDiagnostic diag = clang_getDiagnosticInSet(diagSet, i);
		stringstream sstream;

		CXFile file;
		unsigned line, column, offset;
		clang_getSpellingLocation(clang_getDiagnosticLocation(diag),
			&file, &line, &column, &offset);

		CXString filePath = clang_getFileName(file);
		// if this diagnosis is related to a file
		if(clang_getCString(filePath) != nullptr) {
			string fileName = getFileName(clang_getCString(filePath));
			clang_disposeString(filePath);

			sstream << fileName << ":" << line << ":" << column << " ";
		}

		unsigned formatOption = CXDiagnostic_DisplaySourceRanges |
							  CXDiagnostic_DisplayOption |
							  CXDiagnostic_DisplayCategoryName;
		CXString format = clang_formatDiagnostic(diag, formatOption);
		sstream << clang_getCString(format);
		clang_disposeString(format);

		result.emplace_back(sstream.str());
	}

	return result;
}
