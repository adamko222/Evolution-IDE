#ifndef COMPILEDOCK_H
#define COMPILEDOCK_H

#include <QDockWidget>

#include <QTabWidget>
#include <QPlainTextEdit>
#include <QWidget>

#include "customtabstyle.h"

/*

  dock->OuterTab[0]->InnerTab[0]-> Logs; InnerTab[1]-> Compiler Output; InnerTab[2]-> Raw Output
        OuterTab[1]-> find, search window

*/

class CompileDock : public QDockWidget
{
    Q_OBJECT
public:
    CompileDock(QWidget *parent = nullptr);

private:
    QTabWidget *OuterTab;

    QTabWidget *InnerTab;
    CustomTabStyle *style; // innertab style -> horizontal and right(west) aligned
    QPlainTextEdit *Logs;
    QPlainTextEdit *CompileOutput;
    QPlainTextEdit *RawCompileOutput;

    QWidget *SearchResults;
};

#endif // COMPILEDOCK_H
