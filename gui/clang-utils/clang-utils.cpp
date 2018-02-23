#include <string>
#include <sstream>
#include <vector>

#include <QFile> // for getFileName
#include <QFileInfo>

#include "./clang-utils.h"

using namespace std;

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

/// from clang string to c++ std::string and dispose clang string;
string convertStr(CXString&& clangStr) {
	string result(clang_getCString(clangStr));
	clang_disposeString(clangStr);
	return result;
}

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
		string fileName = getFileName(clang_getCString(filePath));
		clang_disposeString(filePath);

		sstream << fileName << ":" << line << ":" << column << " ";

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
