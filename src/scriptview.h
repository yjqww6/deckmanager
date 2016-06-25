#ifndef SCRIPTVIEW_H
#define SCRIPTVIEW_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QFile>
#include <QVBoxLayout>
#include "card.h"
#include "deckmodel.h"
#include <QDebug>

class ScriptView : public QDialog
{
    Q_OBJECT
public:
    ScriptView(QWidget *parent = nullptr);
    void showEvent(QShowEvent *event)
    {
        m_textEdit->updateGeometry();
        QDialog::showEvent(event);
    }

    static const int NORMAL = 0;
    static const int BRACKET = 1;
    static const int COUNT = 2;
public slots:
    void setId(quint32 id);
    void setDeck(DeckModel *model, bool hideSide, int mode);
private:
    QPlainTextEdit *m_textEdit;
};

#endif // SCRIPTVIEW_H
