#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <vector>

#include "./clang-utils/clang-utils.h"

#ifndef HIGH_LIGHTER_HEADER
#define HIGH_LIGHTER_HEADER

class QTextDocument;

class HighlighRule {
public:
	int start, end;
	QTextCharFormat format;
	HighlighRule(int start, int end, QTextCharFormat format):
		start(start), end(end), format(format) {}
};

class Highlighter : public QSyntaxHighlighter {

private:
	std::vector<HighlighRule> fgRules;
	std::vector<HighlighRule> styleRules;
	std::vector<HighlighRule> bgRules;
	std::vector<HighlighRule> merged;

public:
	Highlighter(QTextDocument *parent = nullptr): QSyntaxHighlighter(parent) {}

	void clearFgRules() { fgRules.clear(); }
	void clearStyleRules() { styleRules.clear(); }
	void clearBgRules() { bgRules.clear(); }

	void addFgRule(HighlighRule rule) {
		fgRules.emplace_back(rule); }
	void addFgRules(const std::vector<HighlighRule>& rules) {
		fgRules.insert(fgRules.end(), rules.begin(), rules.end()); }

	void addStyleRule(HighlighRule rule) {
		styleRules.emplace_back(rule); }
	void addStyleRules(const std::vector<HighlighRule>& rules) {
		styleRules.insert(styleRules.end(), rules.begin(), rules.end()); }

	void addBgRule(HighlighRule rule) {
		bgRules.emplace_back(rule); }
	void addBgRules(const std::vector<HighlighRule>& rules) {
		bgRules.insert(bgRules.end(), rules.begin(), rules.end()); }


	static QColor& getFgColorFromToken(const MyTokenizeNode& token,
		char lastIsMacroChar, char *thisIsMacroChar);

	void addFgRulesFromTokens(const std::vector<MyTokenizeNode>& tokens);

protected:
    void highlightBlock(const QString &text) override;
};

#endif // HIGH_LIGHTER_HEADER
