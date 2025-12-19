#include "idatabase.h"
#include <QUuid>
#include <QDebug>
#include <QDateTime>
#include <QItemSelectionModel>


void IDatabase::ininDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE"); // 添加SQLITE数据库驱动
    QString aFile = "D:/Qt codes/lab4a.db";
    database.setDatabaseName(aFile); // 设置数据库名称

    if (!database.open()) { // 打开数据库
        qDebug() << "failed to open database";
    } else
        qDebug() << "open database is ok" << database.connectionName();
}

bool IDatabase::initPatientModel()
{
    patientTabModel = new QSqlTableModel(this, database);
    patientTabModel->setTable("patient");
    patientTabModel->setEditStrategy(
        QSqlTableModel::OnManualSubmit);//数据保存方式：手动提交
    patientTabModel->setSort(patientTabModel->fieldIndex("name"), Qt::AscendingOrder); // 按name升序排序
    if (!(patientTabModel->select()))// 查询数据
        return false;

    thePatientSelection = new QItemSelectionModel(patientTabModel);
    return true;
}

int IDatabase::addNewPatient()
{
    patientTabModel->insertRow(patientTabModel->rowCount(),
                               QModelIndex()); // 在末尾添加一个记录
    QModelIndex curIndex = patientTabModel->index(patientTabModel->rowCount() - 1,
                                                  1); // 创建最后一行的ModelIndex

    int curRecNo = curIndex.row();
    QSqlRecord  curRec = patientTabModel->record(curRecNo); // 获取当前记录
    curRec.setValue("CREATEDTIMESTAMP", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    curRec.setValue("ID", QUuid::createUuid().toString(QUuid::WithoutBraces));
    patientTabModel->setRecord(curRecNo, curRec);

    return curIndex.row();
}

bool IDatabase::searchPatient(QString filter)
{
    patientTabModel->setFilter(filter);
    return patientTabModel->select();
}

bool IDatabase::deleteCurrentPatient()
{
    if (!patientTabModel || !thePatientSelection)
        return false;

    QModelIndex curIndex = thePatientSelection->currentIndex();
    if (!curIndex.isValid())
        return false;

    int row = curIndex.row();
    bool ok = patientTabModel->removeRow(row);
    if (!ok)
        return false;

    QSqlDatabase db = patientTabModel->database();
    db.transaction();
    ok = patientTabModel->submitAll();
    if (!ok) {
        patientTabModel->revertAll();
        db.rollback();
        return false;
    }
    db.commit();

    patientTabModel->select();

    int rows = patientTabModel->rowCount();
    if (rows > 0) {
        int newRow = qMin(row, rows - 1);
        QModelIndex newIdx = patientTabModel->index(newRow, 0);
        thePatientSelection->setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    } else {
        thePatientSelection->clear();
    }

    return true;
}

bool IDatabase::submitPatientEdit()
{
    return patientTabModel->submitAll();
}

void IDatabase::revertPatientEdit()
{
    patientTabModel->revertAll();
}

QString IDatabase::userLogin(QString userName, QString password)
{
    // 当前直接返回登录成功，实际逻辑被注释
    //return "loginOK";

    //原本的登录验证逻辑（被注释）
    QSqlQuery query;
    query.prepare("select username, password from user where username = :USER");
    query.bindValue(":USER", userName);
    query.exec();
    qDebug() << query.lastQuery() << query.first();

    if (query.first() && query.value("username").isValid()) {
        QString passwd = query.value("password").toString();
        if (passwd == password) {
            qDebug() << "login ok";
            return "loginOk";
        } else {
            qDebug() << "wrong password";
            return "wrongPassword";
        }
    } else {
        qDebug() << "no such user";
        return "wrongUsername";
    }
}


IDatabase::IDatabase(QObject *parent): QObject(parent){
    ininDatabase();
}
