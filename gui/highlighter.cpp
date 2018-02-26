#include <set>

#include <clang-c/Index.h>

#include "./highlighter.h"
#include "./color-scheme.h"

QColor& Highlighter::getFgColorFromToken(const MyTokenizeNode& token,
	char lastIsMacroChar, char* thisIsMacroChar) {

	auto type = token.type;
	char firstChar = token.name[0];

	if(lastIsMacroChar) {
		type = CXToken_Keyword;
		*thisIsMacroChar = 0;
	} else if(token.name == "#" && token.col0 == 1) {
		type = CXToken_Keyword;
		*thisIsMacroChar = 1;
	} else {
		*thisIsMacroChar = 0;
	}

	switch(type) {
	case CXToken_Comment: return ColorScheme::COLOR_COMMENT;
	case CXToken_Keyword: return ColorScheme::COLOR_KEYWORD;
	case CXToken_Literal:
		return firstChar == '"' || firstChar == '\''
			? ColorScheme::COLOR_LITERAL_STR
			: ColorScheme::COLOR_LITERAL_NUM;
	default:
		return ColorScheme::COLOR_NORMAL;
	}
}

void Highlighter::addFgRulesFromTokens(const std::vector<MyTokenizeNode> &tokens) {
	char lastIsMacroChar = 0, thisIsMacroChar = 0;
	for(const MyTokenizeNode& token: tokens) {
		auto format = ColorScheme::asForegroundFormat(
			getFgColorFromToken(token, lastIsMacroChar, &thisIsMacroChar));
		this->addFgRule(HighlighRule(token.begin, token.end, std::move(format)));
		lastIsMacroChar = thisIsMacroChar;
	}
}

void Highlighter::highlightBlock(const QString&) {
	// default foreground syntax highlight
	for(const auto& rule: fgRules)
		this->setFormat(rule.start, rule.end - rule.start, rule.format);

	// selected ast area to highlight background
	for(const auto& rule: bgRules)
		this->setFormat(rule.start, rule.end - rule.start, rule.format);

	// selected token
	for(const auto& rule: styleRules)
		this->setFormat(rule.start, rule.end - rule.start, rule.format);
}


