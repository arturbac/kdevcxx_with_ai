/********************************************************************************
** Form generated from reading UI file 'cxx_with_gpt_settings.ui'
**
** Created by: Qt User Interface Compiler version 5.15.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CXX_WITH_GPT_SETTINGS_H
#define UI_CXX_WITH_GPT_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CxxWithGptSettings
  {
public:
  QVBoxLayout * verticalLayout;
  QLabel * labelAiKey;
  QLineEdit * lineEditAiKey;
  QLabel * labelAiEndpoint;
  QLineEdit * lineEditAiEndpoint;

  void setupUi(QWidget * CxxWithGptSettings)
    {
    if(CxxWithGptSettings->objectName().isEmpty())
      CxxWithGptSettings->setObjectName(QString::fromUtf8("CxxWithGptSettings"));
    verticalLayout = new QVBoxLayout(CxxWithGptSettings);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    labelAiKey = new QLabel(CxxWithGptSettings);
    labelAiKey->setObjectName(QString::fromUtf8("labelAiKey"));

    verticalLayout->addWidget(labelAiKey);

    lineEditAiKey = new QLineEdit(CxxWithGptSettings);
    lineEditAiKey->setObjectName(QString::fromUtf8("lineEditAiKey"));

    verticalLayout->addWidget(lineEditAiKey);

    labelAiEndpoint = new QLabel(CxxWithGptSettings);
    labelAiEndpoint->setObjectName(QString::fromUtf8("labelAiEndpoint"));

    verticalLayout->addWidget(labelAiEndpoint);

    lineEditAiEndpoint = new QLineEdit(CxxWithGptSettings);
    lineEditAiEndpoint->setObjectName(QString::fromUtf8("lineEditAiEndpoint"));

    verticalLayout->addWidget(lineEditAiEndpoint);

    retranslateUi(CxxWithGptSettings);

    QMetaObject::connectSlotsByName(CxxWithGptSettings);
    }  // setupUi

  void retranslateUi(QWidget * CxxWithGptSettings)
    {
    labelAiKey->setText(QCoreApplication::translate("CxxWithGptSettings", "AI Key:", nullptr));
    labelAiEndpoint->setText(QCoreApplication::translate("CxxWithGptSettings", "AI Endpoint:", nullptr));
    (void)CxxWithGptSettings;
    }  // retranslateUi
  };

namespace Ui
  {
class CxxWithGptSettings : public Ui_CxxWithGptSettings
  {
  };
  }  // namespace Ui

QT_END_NAMESPACE

#endif  // UI_CXX_WITH_GPT_SETTINGS_H
