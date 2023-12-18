#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    arduino = new seriallink;
    arduino->openConnection();

    connect(arduino, &seriallink::gotNewData, this, &MainWindow::updateGUI);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateGUI(QByteArray data){
    ui->lcd_byte_received->display(ui->lcd_byte_received->value() + data.size());
}




void MainWindow::on_p_motorOn_clicked()
{
    if (arduino->isWritable())
        arduino->write("o");
    else
        qDebug() << "Couldn't write to serial!";
}


void MainWindow::on_motorOff_clicked()
{
    if (arduino->isWritable())
        arduino->write("n");
    else
        qDebug() << "Couldn't write to serial!";
}

