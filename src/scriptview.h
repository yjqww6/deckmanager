#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QFile>
#include <QVBoxLayout>
#include "card.h"
#include <QDebug>

class ScriptView : public QDialog
{
    Q_OBJECT
public:
    ScriptView(QWidget *parent = nullptr);
    void showEvent(QShowEvent *event)
    {
        textEdit->updateGeometry();
        QDialog::showEvent(event);
    }

public slots:
    void setId(int id);
private:
    QPlainTextEdit *textEdit;
};

#endif // SCRIPTVIEW_H
