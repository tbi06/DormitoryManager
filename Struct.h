#pragma once
#include <string>
#include <vector>
#include <ctime> // cho kiểu DATE/DATETIME

// ========================================================================
// FIX: Đã thêm giá trị khởi tạo (= 0, = 0.0) cho tất cả các biến số.
// Điều này giúp loại bỏ cảnh báo C26495 (Uninitialized variable).
// ========================================================================

// 1. Bảng Room
struct Room {
    int room_id = 0;
    std::string room_number;
    std::string type;
    int capacity = 0;
    std::string status;
    std::string building;
};

// 2. Bảng Student
struct Student {
    int student_id = 0;
    std::string name;
    std::string email;
    std::string phone;
    std::string gender;
    std::string DOB;       // YYYY-MM-DD
    std::string address;
};

// 3. Bảng DormitoryManager
struct DormitoryManager {
    int manager_id = 0;
    std::string name;
    std::string email;
    std::string phone;
};

// 4. Bảng Contract
struct Contract {
    int contract_id = 0;
    int student_id = 0;
    int room_id = 0;
    std::string start_date; // YYYY-MM-DD
    std::string end_date;   // YYYY-MM-DD
    std::string status;
};

// 5. Bảng Invoice
struct Invoice {
    int invoice_id = 0;
    int contract_id = 0;
    double amount = 0.0;
    std::string due_date; // YYYY-MM-DD
    std::string status;
};

// 6. Bảng MaintenanceRequest
struct MaintenanceRequest {
    int request_id = 0;
    std::string description;
    std::string date; // YYYY-MM-DD
    std::string status;
    int student_id = 0;    // optional
    int manager_id = 0;    // optional
    std::string equip_id;
};

// 7. Bảng UserAccount
struct UserAccount {
    int account_id = 0;
    std::string username;
    std::string password;
    std::string role;
};

// 8. Bảng Admin
struct Admin {
    int admin_id = 0;
    std::string username;
    std::string password;
    std::string privilege;
};

// 9. Bảng BillingSystem
struct BillingSystem {
    int transaction_id = 0;
    int account_id = 0;
    std::string method;
    std::string payment_date; // YYYY-MM-DD hh:mm:ss
};

// 10. Bảng BackupResto
struct BackupResto {
    int backup_id = 0;
    std::string date;     // YYYY-MM-DD
    std::string file_path;
    std::string type;
    int admin_id = 0;          // optional
};

// 11. Bảng SystemLog
struct SystemLog {
    int log_id = 0;
    int user_id = 0;           // optional
    std::string activity;
    std::string datetime;  // YYYY-MM-DD hh:mm:ss
};

// 12. Bảng trung gian Invoice_Billing
struct Invoice_Billing {
    int invoice_id = 0;
    int transaction_id = 0;
};