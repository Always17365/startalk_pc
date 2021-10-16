﻿#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

//#include <QLabel>
#include <QTextBrowser>
#include <QWidget>

class ImageBlock;
class TextBrowser : public QTextBrowser
{
	Q_OBJECT
public:
    explicit TextBrowser(QWidget* parent = nullptr);
    // QWidget interface
public:
    QSize sizeHint() const override;

public:
    void addImage(const QString& imagePath, qreal width, qreal height);
    void addLink(const QString& link, qreal width);
    QTextCursor getCurrentCursor();

Q_SIGNALS:
    void imageClicked(int index);
    void sgImageClicked(const QString&, const QString&);
    void linkClicked(const QString& link);
    void sgClicked();

protected:
    void wheelEvent(QWheelEvent *event) override {};
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

//private:
//    bool _pressed = false;
//    int  _scrollVal = 0;
};

#endif // TEXTBROWSER_H
