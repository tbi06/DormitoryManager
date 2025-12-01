CREATE DATABASE quan_ly_ky_tuc_xa;
USE quan_ly_ky_tuc_xa;

-- 1. Bảng Room (Phòng)
CREATE TABLE Room (
    room_id INT PRIMARY KEY,
    room_number VARCHAR(10) NOT NULL UNIQUE,
    type VARCHAR(50) NOT NULL,
    capacity INT NOT NULL,
    status VARCHAR(20) NOT NULL,
    building VARCHAR(50)
);

-- 2. Bảng Student (Sinh viên)
CREATE TABLE Student (
    student_id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE,
    phone VARCHAR(20),
    gender VARCHAR(10),
    DOB DATE,
    address VARCHAR(255)
);

-- 3. Bảng DormitoryManager (Quản lý Ký túc xá)
CREATE TABLE DormitoryManager (
    manager_id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE,
    phone VARCHAR(20)
);

-- 4. Bảng Contract (Hợp đồng) - Liên kết Student và Room
CREATE TABLE Contract (
    contract_id INT PRIMARY KEY,
    student_id INT NOT NULL,
    room_id INT NOT NULL,
    start_date DATE NOT NULL,
    end_date DATE,
    status VARCHAR(20) NOT NULL,
    FOREIGN KEY (student_id) REFERENCES Student(student_id),
    FOREIGN KEY (room_id) REFERENCES Room(room_id)
);

-- 5. Bảng Invoice (Hóa đơn)
CREATE TABLE Invoice (
    invoice_id INT PRIMARY KEY,
    contract_id INT NOT NULL,
    amount DECIMAL(10, 2) NOT NULL,
    due_date DATE NOT NULL,
    status VARCHAR(20) NOT NULL,
    FOREIGN KEY (contract_id) REFERENCES Contract(contract_id)
);

-- 6. Bảng MaintenanceRequest (Yêu cầu Bảo trì)
CREATE TABLE MaintenanceRequest (
    request_id INT PRIMARY KEY,
    description TEXT NOT NULL,
    date DATE NOT NULL,
    status VARCHAR(20) NOT NULL,
    student_id INT,
    manager_id INT,
    equip_id VARCHAR(50),
    FOREIGN KEY (student_id) REFERENCES Student(student_id),
    FOREIGN KEY (manager_id) REFERENCES DormitoryManager(manager_id)
);

-- 7. Bảng UserAccount (Tài khoản Người dùng)
CREATE TABLE UserAccount (
    account_id INT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL
);

-- 8. Bảng Admin (Quản trị viên)
CREATE TABLE Admin (
    admin_id INT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    privilege VARCHAR(50),
    FOREIGN KEY (admin_id) REFERENCES UserAccount(account_id)
);

-- 9. Bảng BillingSystem (Hệ thống Thanh toán/Giao dịch) - Đã loại bỏ invoice_id
CREATE TABLE BillingSystem (
    transaction_id INT PRIMARY KEY,
    account_id INT NOT NULL,
    method VARCHAR(50) NOT NULL,
    payment_date DATETIME NOT NULL,
    FOREIGN KEY (account_id) REFERENCES UserAccount(account_id)
);

-- 10. Bảng BackupResto (Sao lưu/Khôi phục)
CREATE TABLE BackupResto (
    backup_id INT PRIMARY KEY,
    date DATE NOT NULL,
    file_path VARCHAR(255) NOT NULL,
    type VARCHAR(50) NOT NULL,
    admin_id INT,
    FOREIGN KEY (admin_id) REFERENCES Admin(admin_id)
);

-- 11. Bảng SystemLog (Nhật ký Hệ thống)
CREATE TABLE SystemLog (
    log_id INT PRIMARY KEY,
    user_id INT,
    activity VARCHAR(255) NOT NULL,
    datetime DATETIME NOT NULL,
    FOREIGN KEY (user_id) REFERENCES UserAccount(account_id)
);

--- BẢNG PHỤ (JUNCTION TABLE) ---

-- 12. BẢNG TRUNG GIAN N:N: Invoice_Billing (Liên kết Invoice và BillingSystem)
CREATE TABLE Invoice_Billing (
    invoice_id INT NOT NULL,
    transaction_id INT NOT NULL, 
    
    -- Khóa Chính Kép
    PRIMARY KEY (invoice_id, transaction_id),
    
    FOREIGN KEY (invoice_id) REFERENCES Invoice(invoice_id),
    FOREIGN KEY (transaction_id) REFERENCES BillingSystem(transaction_id)
);