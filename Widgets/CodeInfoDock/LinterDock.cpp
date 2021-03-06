#include "LinterDock.h"
#include "icons/IconFactory.h"
#include <QShortcut>

LinterDock::LinterDock(QWidget *parent) : QDockWidget(parent) {
    setWindowTitle("Code Linter");
    setFeatures(AllDockWidgetFeatures);
    setVisible(false);
    createWindow();

    addItem("some item", "filepath", clang::DiagnosticsEngine::Level::Warning);
    addItem("another item", "filepath", clang::DiagnosticsEngine::Level::Error);
}

void LinterDock::createWindow() {
    filePath = new QLabel("file path", this);
    TitleBar = new QToolBar(this);
    Items = new QListWidget(this);

    TitleBar->addWidget(filePath);
    auto *spacer = new QWidget(this);// align to right with blank widget
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    TitleBar->addWidget(spacer);
    TitleBar->addAction(QIcon(IconFactory::Remove), "Close", this, SLOT(close()));
    TitleBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    TitleBar->setFixedHeight(30);
    TitleBar->setOrientation(Qt::Horizontal);

    connect(new QShortcut(Qt::Key_Escape, this), &QShortcut::activated, [=] { setVisible(false); });

    setTitleBarWidget(TitleBar);
    setWidget(Items);
}

void LinterDock::addItem(const QString &content, const QString &filePath,
                         const clang::DiagnosticsEngine::Level &level) const {
    auto *item = new QListWidgetItem();
    //item->listWidget()->setAutoFillBackground(true);
    switch (level) {
        case clang::DiagnosticsEngine::Level::Warning:
            item->setBackground(QColor(Qt::yellow));
            item->setText(content);
            item->setToolTip(filePath);
            Items->addItem(item);
            break;
        case clang::DiagnosticsEngine::Level::Error:
            item->setForeground(QColor(Qt::red));
            item->setText(content);
            item->setToolTip(filePath);
            Items->addItem(item);
            break;
            // TODO: add others
        default:
            item->setText(content);
            item->setToolTip(filePath);
            Items->addItem(item);
            break;
    }
}
