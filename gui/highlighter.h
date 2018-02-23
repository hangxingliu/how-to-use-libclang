#ifndef HIGH_LIGHTER_HEADER
#define HIGH_LIGHTER_HEADER

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <vector>

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
	std::vector<HighlighRule> highlighRules;

public:
	Highlighter(QTextDocument *parent = nullptr): QSyntaxHighlighter(parent) {}
	void clearHighlighRules() { highlighRules.clear(); }
	void addHighlighRule(HighlighRule rule) {
		highlighRules.emplace_back(rule); }
	void addHighlighRules(const std::vector<HighlighRule>& rules) {
		highlighRules.insert(highlighRules.end(), rules.begin(), rules.end()); }

protected:
    void highlightBlock(const QString &text) override;
};

#endif // HIGH_LIGHTER_HEADER
