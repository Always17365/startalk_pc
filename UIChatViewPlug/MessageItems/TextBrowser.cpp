#include "TextBrowser.h"
#include <QDebug>
#include <QTextBlock>

#include "../blocks/LinkBlock.h"
#include "../blocks/ImageBlock.h"
#include "../blocks/block_define.h"
#include "DataCenter/Platform.h"

TextBrowser::TextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    this->setFrameShape(QFrame::NoFrame);
    this->setOpenLinks(false);
    this->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet(QString("background-color:rgba(0,0,0,0)"));
    setContextMenuPolicy(Qt::NoContextMenu);
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.10.17
  */
QSize TextBrowser::sizeHint() const
{
    return QSize(document()->size().width(), document()->size().height() + 2);
}


/**
 *
 * @param imagePath
 */
void TextBrowser::addImage(const QString &imagePath, qreal width, qreal height)
{
    QTextCharFormat imageFromat;
    imageFromat.setFontWordSpacing(0);
    imageFromat.setFontLetterSpacing(0);
    imageFromat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    imageFromat.setObjectType(imageObjectType);
    imageFromat.setProperty(imagePropertyPath, imagePath);
    imageFromat.setProperty(imagePropertyWidth, width);
    imageFromat.setProperty(imagePropertyHeight, height);

    textCursor().insertText(QString(QChar::ObjectReplacementCharacter),
                            imageFromat);
}

/**
 *
 * @param link
 */
void TextBrowser::addLink(const QString &link, qreal width)
{

    QTextCharFormat linkFromat;
    linkFromat.setFontWordSpacing(0);
    linkFromat.setFontLetterSpacing(0);
    linkFromat.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    linkFromat.setObjectType(linkObjectType);
    linkFromat.setProperty(linkPropertyLink, link);
    linkFromat.setProperty(linkPropertyFont, this->font());
    linkFromat.setProperty(linkPropertyWidth, width);
    textCursor().insertText(QString(QChar::ObjectReplacementCharacter), linkFromat);
}

void TextBrowser::mousePressEvent(QMouseEvent *e)
{
    //    _pressed = true;
    //    _scrollVal = this->textCursor().blockNumber();
    emit sgClicked();
    QTextBrowser::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        //        QPoint eventPos = e->pos();
        //        QTextCursor cursor = cursorForPosition(eventPos);
        //        // now check to see if we've moved the cursor to the space
        //        // before or after the actual click location
        //        QRect rect = cursorRect();
        //        if(rect.x() < eventPos.x())
        //            cursor.movePosition(QTextCursor::Right);
        //        // charFormat is for the object BEFORE the
        //        // cursor postion
        QPoint eventPos = e->pos();
        QTextCursor cursor = cursorForPosition(eventPos);
        QRect rect = cursorRect(cursor);

        if (rect.x() < eventPos.x()) {
            cursor.movePosition(QTextCursor::Right);
        }

        auto fmt = cursor.charFormat();
        bool isImage = fmt.isImageFormat();
        int type = fmt.property(imagePropertyType).toInt();

        if (isImage && type == 2) { // StTextMessage::EM_IMAGE
            QString imageLink = fmt.property(imagePropertyLink).toString();
            QString imagePath = fmt.property(imagePropertyPath).toString();

            if (imagePath.isEmpty()) {
                imagePath = fmt.toImageFormat().name();

                if (imagePath == imageLink) {
                    imagePath = st::GetImagePathByUrl(imagePath.toStdString()).data();
                }
            }

            int index = fmt.property(imagePropertyIndex).toInt();
            emit imageClicked(index);
            emit sgImageClicked(imagePath, imageLink);
        }
    }
}


QTextCursor TextBrowser::getCurrentCursor()
{
    QPoint eventPos = mapFromGlobal(QCursor::pos());
    QTextCursor cursor = cursorForPosition(eventPos);
    //    QRect rect = cursorRect(cursor);
    //    if(rect.x() < eventPos.x())
    //        cursor.movePosition(QTextCursor::WordRight);
    return cursor;
}

void TextBrowser::mouseReleaseEvent(QMouseEvent *e)
{
    //    _pressed = false;
    QTextBrowser::mouseReleaseEvent(e);
}
