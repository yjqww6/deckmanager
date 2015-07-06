#include "scriptview.h"
#include "expansions.h"

ScriptView::ScriptView(QWidget *parent)
    : QDialog(parent)
{
    textEdit = new QPlainTextEdit;

    textEdit->setReadOnly(true);
    auto layout = new QVBoxLayout;
    layout->addWidget(textEdit, 1);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(layout);
    resize(640, 480);
}

void ScriptView::setId(quint32 id)
{
    setWindowTitle(cardPool->getCard(id)->name);
    QFile file("script/c" + QString::number(id) + ".lua");
    QString scriptText;
    textEdit->clear();
    if(file.open(QFile::ReadOnly))
    {
        scriptText = file.readAll();
    }
    else
    {
       scriptText = expansions->open("script/c" + QString::number(id) + ".lua");
    }

    textEdit->insertPlainText(scriptText);
    //textEdit->adjustSize();
    if(isHidden())
    {
        show();
    }
}
