#include <QTreeWidgetItem>
#include <QString>
#include <QDebug>

#include <stack>

#include "./clang-utils.h"

#ifndef CLANG_TREEWIDGET_ITEM_HEADER
#define CLANG_TREEWIDGET_ITEM_HEADER

class ClangTreeWidgetItem: public QTreeWidgetItem {
	const MyASTNode* relatedNode;

public:
	ClangTreeWidgetItem(const MyASTNode* relatedNode):
		relatedNode(relatedNode) {}
	ClangTreeWidgetItem(QTreeWidget *parent, const MyASTNode* relatedNode):
		QTreeWidgetItem(parent), relatedNode(relatedNode) {}
	ClangTreeWidgetItem(QTreeWidgetItem *parent, const MyASTNode* relatedNode):
		QTreeWidgetItem(parent), relatedNode(relatedNode) {}

	const MyASTNode* getASTNode() { return relatedNode; }

	void setText(const QString &type, const QString &name) {
		QTreeWidgetItem::setText(0, type);
		QTreeWidgetItem::setText(1, name);
	}

	ClangTreeWidgetItem* parent() const {
		return (ClangTreeWidgetItem*) QTreeWidgetItem::parent();
	}

	void setBackgroundColor(const QColor &color) {
		QTreeWidgetItem::setBackgroundColor(0, color);
		QTreeWidgetItem::setBackgroundColor(1, color);
	}
};

#endif // CLANG_TREEWIDGET_ITEM_HEADER
