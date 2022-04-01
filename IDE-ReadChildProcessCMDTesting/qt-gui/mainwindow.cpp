#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QProcess p;
    QString cmd =
        "E:\\llvm\\UnknownField\\build\\tool\\cli\\Release\\UnknownField-cli.exe E:\\llvm\\UnknownField\\test\\test.cpp";
    p.start(cmd);
    p.waitForFinished();
    QString out = QString::fromStdString(p.readAll().toStdString());
    QString oldout = ui->textEdit->toPlainText();
    QString newout = oldout + "\n" + out;
    ui->textEdit->setText(newout);
}
