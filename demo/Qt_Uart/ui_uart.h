/********************************************************************************
** Form generated from reading UI file 'uart.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UART_H
#define UI_UART_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QPushButton *pushButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QString::fromUtf8("Form"));
        Form->resize(522, 283);
        pushButton = new QPushButton(Form);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(340, 70, 111, 51));
        pushButton->setStyleSheet(QString::fromUtf8("font: 12pt \"Courier New\";\n"
""));
        label = new QLabel(Form);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(80, 140, 101, 41));
        label->setStyleSheet(QString::fromUtf8("font: 12pt \"Courier New\";"));
        label_2 = new QLabel(Form);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 140, 271, 41));
        label_2->setStyleSheet(QString::fromUtf8("font: 12pt \"Courier New\";"));
        label_3 = new QLabel(Form);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(80, 70, 221, 51));
        label_3->setStyleSheet(QString::fromUtf8("\n"
"font: 12pt \"Courier New\";"));

        retranslateUi(Form);

        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QApplication::translate("Form", "Form", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("Form", "SEND", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Form", "RECEIVE:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QString());
        label_3->setText(QApplication::translate("Form", "SEND: hello", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UART_H
