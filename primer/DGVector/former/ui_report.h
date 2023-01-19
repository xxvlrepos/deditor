/********************************************************************************
** Form generated from reading UI file 'report.ui'
**
** Created: Tue Jul 6 14:53:21 2010
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REPORT_H
#define UI_REPORT_H

#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSpacerItem>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

QT_BEGIN_NAMESPACE

#if QT_VERSION < 0x050000
#define   QStringLiteral(a)     QString::fromUtf8(a)
#endif

class Ui_reportClass
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QTextBrowser *textBrowser;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *SaveAsButton;
    QPushButton *PrintButton;

    void setupUi(QWidget *reportClass)
    {
        if (reportClass->objectName().isEmpty()){
            reportClass->setObjectName(QStringLiteral("reportClass"));
        }
        reportClass->setWindowModality(Qt::NonModal);
        reportClass->resize(651, 565);


        verticalLayout = new QVBoxLayout(reportClass);
        verticalLayout->setSpacing(10);
        verticalLayout->setContentsMargins(10, 10, 10, 10);

        textBrowser = new QTextBrowser(reportClass);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setAutoFormatting(QTextEdit::AutoAll);
        textBrowser->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse|Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);

        verticalLayout->addWidget(textBrowser);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        SaveAsButton = new QPushButton(reportClass);
        SaveAsButton->setObjectName(QStringLiteral("SaveAsButton"));

        horizontalLayout->addWidget(SaveAsButton);

        PrintButton = new QPushButton(reportClass);
        PrintButton->setObjectName(QStringLiteral("PrintButton"));

        horizontalLayout->addWidget(PrintButton);
        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(reportClass);

        QMetaObject::connectSlotsByName(reportClass);
    } // setupUi

    void retranslateUi(QWidget *reportClass)
    {
        reportClass->setWindowTitle(QApplication::translate("reportClass", "report", 0));
        SaveAsButton->setText(QApplication::translate("reportClass", "Save As", 0));
        PrintButton->setText(QApplication::translate("reportClass", "Print", 0));
    } // retranslateUi

};

namespace Ui {
    class reportClass: public Ui_reportClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REPORT_H
