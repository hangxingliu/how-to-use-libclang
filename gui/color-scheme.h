#include <vector>
#include <QColor>
#include <QTextCharFormat>

#ifndef COLOR_SCHEME_HEADER
#define COLOR_SCHEME_HEADER

class ColorScheme {
	unsigned index;
	unsigned border;
public:
	static std::vector<QColor> COLOR_VECTOR;
	static QColor getPrimary() { return COLOR_VECTOR[0]; }

	ColorScheme();
	const QColor &next();
	const QColor &last();

	static QTextCharFormat asBackgroundFormat(const QColor& color) {
		QTextCharFormat format;
		format.setBackground(color);
		return format;
	}
};

#endif // COLOR_SCHEME_HEADER
