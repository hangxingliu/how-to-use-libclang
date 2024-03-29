#include <vector>
#include <QMainWindow>
#include <QTreeWidgetItem>

#include <stack>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./highlighter.h"
#include "./clang-utils/clang-treewidget-item.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_actionOpen_triggered();

	void onTreeASTSelectionChanged();
	void onListTokenSelectionChanged();

private:
	Ui::MainWindow *ui;

// ===============================
// Custom >>>>>>>>>>>>

	static const QString DEFAULT_TITLE;

	MyClangParser* parser = nullptr;
	MyASTNode* astRootNode = nullptr;

	std::vector<ClangTreeWidgetItem*> treeItems;
	std::vector<ClangTreeWidgetItem*> lastColoredItems;
	QColor treeItemDefaultBg;
	bool disposingTreeItems = false;
	void disposeTreeItems() {
		disposingTreeItems = true;

		if(!treeItems.empty())
			delete treeItems[0];
		//delete top level item and its children will be deleted automatically
		treeItems.clear();
		lastColoredItems.clear();

		disposingTreeItems = false;
	}

	QString currentFilePath;
	QString currentFileName;

	Highlighter* highlighter = nullptr;

	void setupStatusBar();
	void setupTreeAndList();
	void displaySources(QString sources);

	std::stack<int> getTreeCurrentPath();

	void scrollSourcesTo(int offset);
};

#endif // MAINWINDOW_H
