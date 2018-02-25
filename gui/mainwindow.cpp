#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <stack>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "./highlighter.h"
#include "./color-scheme.h"
#include "./clang-utils/clang-utils.h"

const QString MainWindow::DEFAULT_TITLE = "LibClang AST GUI";

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow) {

	ui->setupUi(this);
	this->setupStatusBar();
	this->setupTreeWidget();

	this->highlighter = new Highlighter(ui->textSources->document());
}

MainWindow::~MainWindow() {
	if(this->highlighter) {
		delete highlighter;
		highlighter = nullptr;
	}
	delete ui;
}

void MainWindow::setupStatusBar() {
	ui->actionOpen->setStatusTip(ui->actionOpen->toolTip());
}

void MainWindow::setupTreeWidget() {
	ui->treeWidget->setColumnCount(2);
	ui->treeWidget->setHeaderLabels(QStringList() << "Type" << "Name");
	ui->treeWidget->setColumnWidth(0, 200); // initialized width
	ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	// cancel default highligh color schema
	QPalette palette;
	palette.setColor(QPalette::Highlight, ColorScheme::getPrimary());
	palette.setColor(QPalette::HighlightedText, ui->treeWidget->palette().color(QPalette::Text));
	treeItemDefaultBg = ui->treeWidget->palette().color(QPalette::Base);
	ui->treeWidget->setPalette(palette);

	connect(ui->treeWidget, SIGNAL(itemSelectionChanged()),
		this, SLOT(onTreeWidgetSelectionChanged()));
}

void MainWindow::displaySources(QString sources) {
	QString htmlTemplate =
		R"html_template(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" /><style type="text/css">
	body { font-family: monospace; font-size: 14px; }
	p, li { white-space: pre-wrap; line-height: 20px; }
	p { margin: 0 !important;
		-qt-block-indent:0 !important;
		text-indent:0px !important;}
</style></head><body><p>%1</p></body></html>)html_template";

	QStringList lines = sources.split("\n");
	QStringList htmlLines;
	for(auto& line: lines)
		htmlLines.push_back(line.toHtmlEscaped() + "<br/>");
	ui->textSources->setHtml(htmlTemplate.arg(htmlLines.join("\n")));

	// reset highlighter
	this->highlighter->clearHighlighRules();
}

std::stack<int> MainWindow::getTreeCurrentPath() {
	auto index = ui->treeWidget->currentIndex();
	std::stack<int> s;
	int row = -1;
	while((row = index.row()) >= 0) {
		s.push(row);
		index = index.parent();
	}
	s.pop();//remove top level element location
	return s;
}

void MainWindow::onTreeWidgetSelectionChanged() {
	// because this event will be emitted when disposing tree items
	// for example: you select a tree item in file A, then you open file B
	if(disposingTreeItems)
		return;

	// remove last colored item
	for(ClangTreeWidgetItem* item: lastColoredItems)
		item->setBackgroundColor(this->treeItemDefaultBg);
	lastColoredItems.clear();

	auto currentItem = (ClangTreeWidgetItem*) ui->treeWidget->currentItem();
	if(currentItem == nullptr)
		return;

	auto path = getTreeCurrentPath();
	if(path.empty())
		return; // top level

	ColorScheme scheme; scheme.next(); // ignore primary color
	while((currentItem = currentItem->parent()) != nullptr) {
		currentItem->setBackgroundColor(scheme.next());
		lastColoredItems.push_back(currentItem);
	}
	scheme.last();


	highlighter->clearHighlighRules();
	auto astVisitor = this->astRootNode;
	while(!path.empty()) {
		astVisitor = astVisitor->children[path.top()];
		HighlighRule rule(astVisitor->begin, astVisitor->end,
			ColorScheme::asBackgroundFormat(scheme.last()));
		highlighter->addHighlighRule(std::move(rule));
		path.pop();
	}
	highlighter->rehighlight();
}

/// click "open" action event
void MainWindow::on_actionOpen_triggered() {
	QString filter = QString("")
		+ "All Support Files (*.c *.cc *.cpp *.h *.hpp);;"
		+ "C/C++ Sources (*.c *.cc *.cpp);;"
		+ "C/C++ Headers (*.h *.hpp)";

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open File"), "", filter);

	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::critical(this, tr("Error"),
			tr("Could not open file:") + "\n  " + fileName);
		return;
	}

	// ==============================
	//       load success
	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	QFileInfo fi(file.fileName());
	this->currentFilePath = fi.filePath();
	this->currentFileName = fi.fileName();

	this->setWindowTitle(currentFileName + " - " + DEFAULT_TITLE);
	this->displaySources(file.readAll());
	ui->statusBar->showMessage(tr("Loaded: ") + fileName);

	if(parser != nullptr) delete parser;
	parser = new MyClangParser(currentFilePath.toStdString(), currentFileName.toStdString());

	if(!parser->isSuccess()) {
		QMessageBox::critical(this, tr("Error"),
			tr("Could not parse file:") + "\n  " + fileName);
		return;
	}

	auto diagnosis = parser->getDiagnosis();
	if(diagnosis.empty()) {
		ui->textLog->setPlainText(fileName + "\n  No diagnosis!");
	} else {
		QStringList list;
		list.append(fileName);
		for(auto& d: diagnosis)
			list.append(QString("  ") + QString::fromStdString(d));
		ui->textLog->setPlainText(list.join("\n"));
	}


	disposeTreeItems();
	auto nodes = parser->getNodesVector();
	this->astRootNode = parser->getASTRootNode();

	for(MyASTNode* node: nodes) {
		ClangTreeWidgetItem* item = nullptr;
		if(node->depth == 0) {
			item = new ClangTreeWidgetItem(ui->treeWidget, node);
			item->setText("Root(File)", this->currentFileName);
		} else {
			item = new ClangTreeWidgetItem(node);
			item->setText(node->type.c_str(), node->name.c_str());
		}
		this->treeItems.push_back(item);
	}
	// link items:
	for(const auto& parent: nodes) {
		auto parentItem = this->treeItems[parent->id];
		for(const auto& child: parent->children)
			parentItem->addChild(this->treeItems[child->id]);
	}
	//expand first level
	ui->treeWidget->expandItem(ui->treeWidget->topLevelItem(0));

}


