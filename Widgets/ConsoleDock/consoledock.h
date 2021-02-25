#ifndef COMPILEDOCK_H
#define COMPILEDOCK_H

#include <QDockWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

#include <QHBoxLayout>

#include "customtabstyle.h"

/*

  dock->OuterTab[0]->InnerTab[0]-> Logs; InnerTab[1]-> Compiler Output; InnerTab[2]-> Raw Output
        OuterTab[1]-> find, search window

*/

class ConsoleDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ConsoleDock(QWidget *parent = nullptr);
    ~ConsoleDock() = default;

    QPlainTextEdit *ConsoleOutput;

    QToolButton *run;
    QToolButton *rerun;// stop and run again
    QToolButton *stop;

private:
    QHBoxLayout *MainLayout;
    QToolBar *title_bar;
    QToolBar *tool_bar;

    void BuildConsole();

public slots:
    void slotScrollUp() const;
    void slotScrollDown() const;
    void slotClearConsole() const;
};

#endif // COMPILEDOCK_H
