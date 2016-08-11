#include "messagesdialog.h"
#include "epsrproject.h"

MessagesDialog::MessagesDialog(MainWindow *parent) : QDialog(parent)
{
    ui.setupUi(this);

    mainWindow_ = parent;

    Qt::WindowFlags flags = Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
}

void MessagesDialog::refreshMessages()
{
    QByteArray messageText = mainWindow_->messageText();
    char *messages = messageText.data();
    ui.textBrowser->setText(messages);
    ui.textBrowser->verticalScrollBar()->setValue(ui.textBrowser->verticalScrollBar()->maximum());
}
