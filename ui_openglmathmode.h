/********************************************************************************
** Form generated from reading UI file 'openglmathmode.ui'
**
** Created by: Qt User Interface Compiler version 5.15.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPENGLMATHMODE_H
#define UI_OPENGLMATHMODE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OpenglMathMode
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *OpenglMathMode)
    {
        if (OpenglMathMode->objectName().isEmpty())
            OpenglMathMode->setObjectName(QString::fromUtf8("OpenglMathMode"));
        OpenglMathMode->resize(400, 300);
        menuBar = new QMenuBar(OpenglMathMode);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        OpenglMathMode->setMenuBar(menuBar);
        mainToolBar = new QToolBar(OpenglMathMode);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        OpenglMathMode->addToolBar(mainToolBar);
        centralWidget = new QWidget(OpenglMathMode);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        OpenglMathMode->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(OpenglMathMode);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        OpenglMathMode->setStatusBar(statusBar);

        retranslateUi(OpenglMathMode);

        QMetaObject::connectSlotsByName(OpenglMathMode);
    } // setupUi

    void retranslateUi(QMainWindow *OpenglMathMode)
    {
        OpenglMathMode->setWindowTitle(QCoreApplication::translate("OpenglMathMode", "OpenglMathMode", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OpenglMathMode: public Ui_OpenglMathMode {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPENGLMATHMODE_H
