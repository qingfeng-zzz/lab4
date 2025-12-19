#include "patienteditview.h"
#include "ui_patienteditview.h"
#include "idatabase.h"
#include <QSqlTableModel>
#include <QSqlQuery>

PatientEditView::PatientEditView(QWidget *parent, int index)
    : QWidget(parent)
    , ui(new Ui::PatientEditView)
{
    ui->setupUi(this);

    ui->dbSpinHeight->setRange(0, 300);
    ui->dbSpinWeight->setRange(0, 500);
    ui->dbCreatedTimeStamp->setReadOnly(true);

    dataMapper = new QDataWidgetMapper();
    QSqlTableModel *tabModel = IDatabase::getInstance().patientTabModel;
    dataMapper->setModel(tabModel); // 绑定病人数据模型
    dataMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit); // 通常需设置提交策略（图片中此处代码不完整）

    connect(ui->dbSpinHeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &PatientEditView::recalcBMI);
    connect(ui->dbSpinWeight, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &PatientEditView::recalcBMI);

    // 绑定界面控件与数据库表字段
    dataMapper->addMapping(ui->dbEditID, tabModel->fieldIndex("ID"));
    dataMapper->addMapping(ui->dbEditName, tabModel->fieldIndex("NAME"));
    dataMapper->addMapping(ui->dbEditIDCard, tabModel->fieldIndex("ID_CARD"));
    dataMapper->addMapping(ui->dbSpinHeight, tabModel->fieldIndex("HEIGHT"));
    dataMapper->addMapping(ui->dbSpinWeight, tabModel->fieldIndex("WEIGHT"));
    dataMapper->addMapping(ui->dbEditMobile, tabModel->fieldIndex("MOBILEPHONE"));
    dataMapper->addMapping(ui->dbDateEditDOB, tabModel->fieldIndex("DOB"), "date");
    dataMapper->addMapping(ui->dbComboSex, tabModel->fieldIndex("SEX"), "currentText");
    dataMapper->addMapping(ui->dbCreatedTimeStamp, tabModel->fieldIndex("CREATEDTIMESTAMP"));

    dataMapper->setCurrentIndex(index); // 定位到指定行的病人记录


}

PatientEditView::~PatientEditView()
{
    delete ui;
}

void PatientEditView::on_pushButton_clicked()
{
    recalcBMI();
    dataMapper->submit();
    IDatabase::getInstance().submitPatientEdit();
    emit goPreviousView();
}


void PatientEditView::on_pushButton_2_clicked()
{
    IDatabase::getInstance().revertPatientEdit();
    emit goPreviousView();
}

void PatientEditView::recalcBMI()
{
    int h = ui->dbSpinHeight->value();
    int w = ui->dbSpinWeight->value();
    if (h <= 0 || w <= 0)
        return;
    double hm = h / 100.0;
    double bmi = w / (hm * hm);
    QSqlTableModel *tabModel = IDatabase::getInstance().patientTabModel;
    int row = dataMapper->currentIndex();
    int bmiCol = tabModel->fieldIndex("BMI");
    if (bmiCol >= 0 && row >= 0) {
        tabModel->setData(tabModel->index(row, bmiCol), bmi, Qt::EditRole);
    } else {
        int idCol = tabModel->fieldIndex("ID");
        if (idCol >= 0 && row >= 0) {
            QString id = tabModel->data(tabModel->index(row, idCol)).toString();
            if (!id.isEmpty()) {
                QSqlQuery q(tabModel->database());
                q.prepare("UPDATE patient SET BMI = :bmi WHERE ID = :id");
                q.bindValue(":bmi", bmi);
                q.bindValue(":id", id);
                q.exec();
            }
        }
    }
}

