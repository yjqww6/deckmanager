#ifndef STRINGRECV_H
#define STRINGRECV_H

#include <QObject>
#include <QString>

class StringRecv : public QObject
{
    Q_OBJECT
public:
    explicit StringRecv(QObject *parent = 0);
    QString text;
signals:

public slots:
    void setText(QString _text)
    {
        text = _text;
    }
};

#endif // STRINGRECV_H
