//
// Created by cc on 2019-02-28.
//
#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_CODESHELL_H
#define STALK_V2_CODESHELL_H

#include <QFrame>
#include <QComboBox>
#include <QLabel>

/**
* @description: CodeShell
* @author: cc
* @create: 2019-02-28 11:44
**/
class QVBoxLayout;
class CodeShell : public QFrame
{
    Q_OBJECT
public:
    explicit CodeShell(const QString &title, QWidget *contentWgt, QWidget *parent = nullptr);

    ~CodeShell() override;

public:
    inline QVBoxLayout *getMainLay()
    {
        return  _pMainLay;
    }
    inline QWidget *getTitleFrm()
    {
        return _pTitleFrm;
    };
    QString getCodeStyle();
    QString getCodeLanguage();
    bool setCodeStyle(const QString &style);
    bool setCodeLanguage(const QString &language);

Q_SIGNALS:
    void closeWnd();
    void styleChanged(const QString &, const QString &);

private:
    void initUi();

protected:
    QComboBox *_pCodeStyle;
    QComboBox *_pCodeLanguage;
    QLabel    *_pTitleLabel;
    QVBoxLayout *_pMainLay;

protected:
    QWidget   *_pContentWgt{ nullptr };
    QString    _title;
    QWidget   *_pTitleFrm {nullptr};
};


#endif //STALK_V2_CODESHELL_H
