/********************************************************************************
** Form generated from reading UI file 'led.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LED_H
#define UI_LED_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_led
{
public:
    QLabel *label_2;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_5;
    QLabel *label_3;

    void setupUi(QWidget *led)
    {
        if (led->objectName().isEmpty())
            led->setObjectName(QString::fromUtf8("led"));
        led->resize(400, 300);
        label_2 = new QLabel(led);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(30, 60, 51, 31));
        pushButton_2 = new QPushButton(led);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(150, 60, 51, 31));
        pushButton_3 = new QPushButton(led);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(150, 180, 51, 31));
        pushButton_4 = new QPushButton(led);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(260, 180, 51, 31));
        pushButton_5 = new QPushButton(led);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));
        pushButton_5->setGeometry(QRect(260, 60, 51, 31));
        label_3 = new QLabel(led);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(33, 180, 51, 31));

        retranslateUi(led);

        QMetaObject::connectSlotsByName(led);
    } // setupUi

    void retranslateUi(QWidget *led)
    {
        led->setWindowTitle(QApplication::translate("led", "led", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("led", "LED_D6", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("led", "on", 0, QApplication::UnicodeUTF8));
        pushButton_3->setText(QApplication::translate("led", "on", 0, QApplication::UnicodeUTF8));
        pushButton_4->setText(QApplication::translate("led", "off", 0, QApplication::UnicodeUTF8));
        pushButton_5->setText(QApplication::translate("led", "off", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("led", "BEEP", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class led: public Ui_led {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LED_H
