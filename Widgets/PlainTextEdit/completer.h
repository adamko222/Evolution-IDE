#ifndef COMPLETER_H
#define COMPLETER_H

#include <QCompleter>
#include <QString>
#include <iostream>
#include <string>
#include <vector>

#include <QListWidget>
//#include <QComboBox>

#include "Delegate.h"

class CompleterItemDelegate : public QStyledItemDelegate {
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        option->backgroundBrush.setColor(QColor(Qt::red));
        option->palette.setBrush(QPalette::Text, QColor(Qt::blue));
        option->palette.setBrush(QPalette::HighlightedText, QColor(Qt::red));
        //option->displayAlignment = Qt::AlignCenter;
        QStyledItemDelegate::initStyleOption(option, index);
    }
};

class Completer : public QCompleter {
Q_OBJECT
public:
    explicit Completer(QObject *parent = nullptr);

    ~Completer() = default;

    // TODO: add icon and type
    void addItems(const std::string &name, const std::string &returnType, const QIcon &icon = QIcon());

    void addItems(const QStringList &data);

    // clear the completions data, NOTE! remains the default ones
    void clear();

    // default data for certain file extension, type meant to be as default.
    QStringList defaultCompletionData;

private:
    QListWidget *Popup;


    // words pairs to macth different name for suggested symbol
    // there is loop running with them and actual data to compare similar names
    // really helps sometime
    const QList<QPair<QString, QString>> pairs {
        QPair{"get", "load"},
        QPair{"set", "add"},
        QPair{"remove", "take"},
        QPair{"remove", "delete"},
        QPair{"clear", "reset"},
        QPair{"do", "run"}
        // TODO: add some other what comes to your mind
    };


};

#endif // COMPLETER_H
