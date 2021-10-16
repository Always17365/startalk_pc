//
// Created by cc on 18-12-9.
//

#ifndef STALK_V2_QUOTEBLOCK_H
#define STALK_V2_QUOTEBLOCK_H

#include <QObject>
#include <QTextObjectInterface>
#include <QPainter>
#include "Util/ui/StyleDefine.h"

enum {
    quotePropertySource = QTextFormat::UserProperty + 1,
    quotePropertyFont,
    quotePropertyName,
    quotePropertyText
};


// 显示引用消息的块
class QuoteBlock : public QObject, public QTextObjectInterface
{
	Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    QuoteBlock()
    {

    }

public:
    virtual QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                                 const QTextFormat &format)
    {
		Q_UNUSED(posInDocument)
		Q_UNUSED(doc)

        QString text = format.property(quotePropertyText).toString();
		QString name = format.property(quotePropertyName).toString();
        QFont font = format.property(quotePropertyFont).value<QFont>();
        QFontMetricsF f(font);
        return QSizeF(qMax(f.width(text), f.width(name)) + 10 + 20, f.height() * 2 + 5);
    }
    virtual void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                            int posInDocument, const QTextFormat &format)
    {
        QString name = format.property(quotePropertyName).toString();
        QString text = format.property(quotePropertyText).toString();
        text = name + ": \n" + text;
        painter->save();
        painter->fillRect(rect, QBrush(st::StyleDefine::instance().getQuoteBlockBackgroundColor()));
        painter->fillRect(QRect(rect.x(), rect.y(), 8, rect.height()), QBrush(st::StyleDefine::instance().getQuoteBlockTipColor()));
        painter->setPen(st::StyleDefine::instance().getQuoteBlockFontColor());
        painter->drawText(QRect(rect.x() + 14, rect.y(), rect.width(), rect.height()), Qt::AlignVCenter, text);
        painter->restore();
    }

private:

};

#endif //STALK_V2_QUOTEBLOCK_H
