#include "./color-scheme.h"

using namespace std;

vector<QColor> ColorScheme::COLOR_VECTOR = {
	// Reference from: https://godbolt.org/
	// Colour scheme
	QColor("#D4EFEB"),
	QColor("#FFFFE6"),
	QColor("#E8E7F1"),
	QColor("#FFD2CF"),
	QColor("#D0E3EF"),
	QColor("#FFE4CB"),
	QColor("#E3F3CE"),
	QColor("#FFEDF6"),
	QColor("#F1F1F1"),
	QColor("#E8D2E7"),
	QColor("#ECF7EB"),
	QColor("#FFF8D1")
};

QColor ColorScheme::COLOR_NORMAL = QColor("#000000");
QColor ColorScheme::COLOR_KEYWORD = QColor("#0000ff");
QColor ColorScheme::COLOR_COMMENT = QColor("#008000");
QColor ColorScheme::COLOR_LITERAL_STR = QColor("#a31515");
QColor ColorScheme::COLOR_LITERAL_NUM = QColor("#09885a");

ColorScheme::ColorScheme():
	index(0), border((unsigned) COLOR_VECTOR.size()) {}

const QColor& ColorScheme::next() {
	if(index >= border)
		index = 0;
	return COLOR_VECTOR[index++];
}

const QColor& ColorScheme::last() {
	if(index == 0)
		index = border;
	return COLOR_VECTOR[--index];
}
