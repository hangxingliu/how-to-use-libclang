#include <stdio.h>
#include <string.h>

#include <string>

#include <clang-c/Index.h>

static const char* DEFAULT_FILE = "../sample_files/1.cc";

class VisitorPackage {
public: 
	int depth = 0;
	CXTranslationUnit unit = nullptr;
	VisitorPackage(int depth, CXTranslationUnit unit): depth(depth), unit(unit) {}
	VisitorPackage goDeeper() const { return VisitorPackage(depth + 1, unit); }
	void printIndent() { for(int i = 0 ; i < depth ; i ++ ) printf("  "); }
};

void printCXCursorToken(CXTranslationUnit unit, CXCursor cursor) {
	CXSourceRange range = clang_getCursorExtent(cursor);
    CXToken *tokens = nullptr;
    unsigned nTokens = 0;
    clang_tokenize(unit, range, &tokens, &nTokens);
	printf("token: [");
    for (unsigned i = 0; i < nTokens; i++) {
        CXString spelling = clang_getTokenSpelling(unit, tokens[i]);
        printf("%s\"%s\"", (i == 0 ? "" : ", "), clang_getCString(spelling));
        clang_disposeString(spelling);
    }
	printf("]");
    clang_disposeTokens(unit, tokens, nTokens);
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor, CXClientData _package) {
	auto package = (VisitorPackage*) _package;

	CXSourceLocation location = clang_getCursorLocation(cursor);
	// exclude children not located in main file
	if(!clang_Location_isFromMainFile(location))
		return CXChildVisit_Continue;

	auto cursorKind = clang_getCursorKind(cursor);
	auto nameClangStr = clang_getCursorSpelling(cursor);
	auto kindClangStr = clang_getCursorKindSpelling(cursorKind);

	package->printIndent();
	printf("%s: \"%s\" ", 
		clang_getCString(kindClangStr), 
		clang_getCString(nameClangStr));

	clang_disposeString(kindClangStr);
	clang_disposeString(nameClangStr);

	printCXCursorToken(package->unit, cursor);
	printf("\n");

	auto deeperPackage = package->goDeeper();
	clang_visitChildren(cursor, visitor, &deeperPackage);

	return CXChildVisit_Continue;
}

int main(int argc, char** argv) {
	const char* file = DEFAULT_FILE;
	if(argc <= 1) {
		printf("use default file: \"%s\"\n", DEFAULT_FILE);
	} else {
		file = argv[1];
	}

	const int clangArgc = 1;
	const char* clangArgs[clangArgc];
	clangArgs[0] = "-std=c++11";

	auto clangIndex = clang_createIndex(1, 1);
	auto clangUnit = clang_parseTranslationUnit(
		clangIndex, //CXIndex
		file,  //source_filename
		clangArgs, clangArgc, // Command line arguments, length
		nullptr, 0, // CXUnsavedFile files, length
		0); // options

	if(clangUnit == nullptr) {
		printf("\n error: could not parse \"%s\"\n", file);
		return 1;
	}

	VisitorPackage initPackage(0, clangUnit);
	auto cursor = clang_getTranslationUnitCursor(clangUnit);
	clang_visitChildren(cursor, visitor, &initPackage);

	return 0;
}