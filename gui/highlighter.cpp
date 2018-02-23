#include "highlighter.h"

void Highlighter::highlightBlock(const QString&) {
	for(const auto& rule: highlighRules) {
		this->setFormat(rule.start, rule.end - rule.start, rule.format);
	}
}


