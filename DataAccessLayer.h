#pragma once
#include "Struct.h"
#include <windows.h>
#include <sqlext.h>
#include <string>
#include <vector>

class DataAccessLayer {
private:
    SQLHANDLE hEnv;
    SQLHANDLE hConn;

    void showError(unsigned int handleType, const SQLHANDLE& handle);

public:
    DataAccessLayer();
    ~DataAccessLayer();
    
    bool connect();
    void disconnect();

    // ------------------ Student ------------------
    bool addStudent(const Student& s);
    bool updateStudent(const Student& s);
    bool deleteStudent(int student_id);
    std::vector<Student> getAllStudents();
    Student getStudentByID(int student_id);

    // ------------------ Room ------------------
    bool addRoom(const Room& r);
    bool updateRoom(const Room& r);
    bool deleteRoom(int room_id);
    std::vector<Room> getAllRooms();
    Room getRoomByID(int room_id);

    // ------------------ DormitoryManager ------------------
    bool addManager(const DormitoryManager& m);
    bool updateManager(const DormitoryManager& m);
    bool deleteManager(int manager_id);
    std::vector<DormitoryManager> getAllManagers();
    DormitoryManager getManagerByID(int manager_id);

    // ------------------ Contract ------------------
    bool addContract(const Contract& c);
    bool updateContract(const Contract& c);
    bool deleteContract(int contract_id);
    Contract getContractByID(int contract_id);
    std::vector<Contract> getAllContracts();

    // ------------------ Invoice ------------------
    bool addInvoice(const Invoice& i);
    bool updateInvoice(const Invoice& i);
    bool deleteInvoice(int invoice_id);
    Invoice getInvoiceByID(int invoice_id);
    std::vector<Invoice> getAllInvoices();

    // ------------------ MaintenanceRequest ------------------
    bool addMaintenanceRequest(const MaintenanceRequest& mr);
    bool updateMaintenanceRequest(const MaintenanceRequest& mr);
    bool deleteMaintenanceRequest(int request_id);
    MaintenanceRequest getMaintenanceRequestByID(int request_id);
    std::vector<MaintenanceRequest> getAllMaintenanceRequests();

    // ------------------ UserAccount ------------------
    bool addUser(const UserAccount& u);
    bool updateUser(const UserAccount& u);
    bool deleteUser(int account_id);
    UserAccount getUserByID(int account_id);
    std::vector<UserAccount> getAllUsers();

    // ------------------ Admin ------------------
    bool addAdmin(const Admin& a);
    bool updateAdmin(const Admin& a);
    bool deleteAdmin(int admin_id);
    Admin getAdminByID(int admin_id);
    std::vector<Admin> getAllAdmins();

    // ------------------ BillingSystem ------------------
    bool addTransaction(const BillingSystem& b);
    bool updateTransaction(const BillingSystem& b);
    bool deleteTransaction(int transaction_id);
    BillingSystem getTransactionByID(int transaction_id);
    std::vector<BillingSystem> getAllTransactions();

    // ------------------ BackupResto ------------------
    bool addBackup(const BackupResto& b);
    bool updateBackup(const BackupResto& b);
    bool deleteBackup(int backup_id);
    BackupResto getBackupByID(int backup_id);
    std::vector<BackupResto> getAllBackups();

    // ------------------ SystemLog ------------------
    bool addLog(const SystemLog& log);
    std::vector<SystemLog> getAllLogs();

    // ------------------ Invoice_Billing (bảng trung gian) ------------------
    bool addInvoiceBilling(const Invoice_Billing& ib);
    bool deleteInvoiceBilling(int invoice_id, int transaction_id);
    std::vector<Invoice_Billing> getAllInvoiceBilling();
};
