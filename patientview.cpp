#include "patientview.h"
#include "ui_patientview.h"
#include "idatabase.h"

PatientView::PatientView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PatientView)
{
    ui->setupUi(this);

    // 表格视图交互规则配置
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows); // 选择整行
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止直接编辑表格
    ui->tableView->setAlternatingRowColors(true); // 交替行颜色（提升可读性）

    // 绑定数据库的病人数据模型与选择模型
    IDatabase &iDatabase = IDatabase::getInstance();
    if (iDatabase.initPatientModel()) {
        ui->tableView->setModel(iDatabase.patientTabModel); // 关联数据模型
        ui->tableView->setSelectionModel(iDatabase.thePatientSelection); // 关联选择模型
    }

}

PatientView::~PatientView()
{
    delete ui;
}

void PatientView::on_btAdd_clicked()
{
    // 新增病人记录并获取行号
    int currow = IDatabase::getInstance().addNewPatient();
    // 发射信号跳转到病人编辑视图
    emit goPatientEditView(currow);
}


void PatientView::on_btSearch_clicked()
{
    // 构造姓名模糊搜索条件
    QString filter = QString("name like '%%1%'").arg(ui->txtSearch->text());
    // 执行病人搜索
    IDatabase::getInstance().searchPatient(filter);
}


void PatientView::on_btDelete_clicked()
{
    IDatabase::getInstance().deleteCurrentPatient();
}


void PatientView::on_btEdit_clicked()
{
    // 获取当前选中行的模型索引
    QModelIndex curIndex = IDatabase::getInstance().thePatientSelection->currentIndex();
    // 发射信号跳转到病人编辑视图（传递选中行号）
    emit goPatientEditView(curIndex.row());
}

