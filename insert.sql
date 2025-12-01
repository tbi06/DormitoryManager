-- ***************************************************************
-- FILE: data_insert.sql
-- MÔ TẢ: Chèn dữ liệu mẫu (INSERT INTO) cho các bảng đã định nghĩa
-- trong cơ sở dữ liệu quan_ly_ky_tuc_xa.
-- *Lưu ý: Các ID được chèn thủ công để đảm bảo tính nhất quán giữa các bảng.*
-- ***************************************************************

-- Dữ liệu này được thiết kế để test các trường hợp sau:
-- 1. Tài khoản đăng nhập cho 3 vai trò (Admin, Manager, Student).
-- 2. Phòng đã có người ở (Room A101) và phòng còn trống (Room B201).
-- 3. Hợp đồng đang hoạt động (Contract 1, 2) và Hợp đồng đã hết hạn (Contract 3).
-- 4. Hóa đơn đang chờ thanh toán (Invoice 1) và đã thanh toán (Invoice 2).
-- 5. Yêu cầu bảo trì đang chờ xử lý.

-- =======================================================================
-- I. Dữ Liệu cho Bảng UserAccount (Tài khoản Người dùng)
-- account_id 1: Admin
-- account_id 2: Manager
-- account_id 3, 4, 5: Student
-- =======================================================================
USE quan_ly_ky_tuc_xa;
INSERT INTO UserAccount (account_id, username, password, role) VALUES
(1, 'admin', 'HASH_admin_123_SALT', 'Admin'),
(2, 'manager_a', 'HASH_manager_456_SALT', 'DormitoryManager'),
(3, 'sinhvien_a', 'HASH_sv_a_SALT', 'Student'),
(4, 'sinhvien_b', 'HASH_sv_b_SALT', 'Student'),
(5, 'sinhvien_c', 'HASH_sv_c_SALT', 'Student');

-- =======================================================================
-- II. Dữ Liệu cho Bảng Admin (Liên kết với UserAccount 1)
-- =======================================================================
INSERT INTO Admin (admin_id, username, password, privilege) VALUES
(1, 'admin', 'HASH_admin_123_SALT', 'FullAccess');

-- =======================================================================
-- III. Dữ Liệu cho Bảng DormitoryManager (Liên kết với UserAccount 2)
-- =======================================================================
INSERT INTO DormitoryManager (manager_id, name, email, phone) VALUES
(2, 'Le Van Hung', 'hunglv@ktxa.edu.vn', '0901234567');

-- =======================================================================
-- IV. Dữ Liệu cho Bảng Student (Liên kết với UserAccount 3, 4, 5)
-- *LƯU Ý: student_id ở đây chính là account_id tương ứng trong UserAccount*
-- =======================================================================
INSERT INTO Student (student_id, name, email, phone, gender, DOB, address) VALUES
(3, 'Nguyen Thi Huong', 'huongnt@hcmut.edu.vn', '0912340003', 'Female', '2003-10-10', 'TP.HCM'),
(4, 'Tran Minh Vu', 'vutm@hcmut.edu.vn', '0912340004', 'Male', '2002-05-20', 'Da Nang'),
(5, 'Pham Thi Mai', 'maipt@hcmut.edu.vn', '0912340005', 'Female', '2001-08-01', 'Ha Noi');

-- =======================================================================
-- V. Dữ Liệu cho Bảng Room (Phòng)
-- =======================================================================
INSERT INTO Room (room_id, room_number, type, capacity, status, building) VALUES
(101, 'A101', 'Standard', 4, 'Occupied', 'A'), -- Đã có người ở
(102, 'A102', 'VIP', 2, 'Available', 'A'),      -- Còn trống
(201, 'B201', 'Standard', 4, 'Available', 'B'), -- Còn trống
(301, 'C301', 'Standard', 4, 'Maintenance', 'C'); -- Bảo trì

-- =======================================================================
-- VI. Dữ Liệu cho Bảng Contract (Hợp đồng)
-- =======================================================================
INSERT INTO Contract (contract_id, student_id, room_id, start_date, end_date, status) VALUES
(1, 3, 101, '2025-09-01', '2026-06-30', 'Active'), -- HĐ hoạt động (SV Huong)
(2, 4, 101, '2025-10-01', '2025-12-31', 'Active'), -- HĐ hoạt động (SV Vu)
(3, 5, 201, '2024-01-01', '2024-06-30', 'Expired'); -- HĐ đã hết hạn (SV Mai - Dữ liệu cũ)

-- =======================================================================
-- VII. Dữ Liệu cho Bảng Invoice (Hóa đơn)
-- =======================================================================
INSERT INTO Invoice (invoice_id, contract_id, amount, due_date, status) VALUES
(1, 1, 650000.00, '2025-11-25', 'Pending'), -- HĐ 1 (Huong): Chờ thanh toán (Test UC12)
(2, 2, 550000.00, '2025-10-20', 'Paid'),    -- HĐ 2 (Vu): Đã thanh toán
(3, 1, 600000.00, '2025-10-25', 'Paid');    -- HĐ 1 (Huong): Đã thanh toán kỳ trước

-- =======================================================================
-- VIII. Dữ Liệu cho Bảng BillingSystem (Giao dịch Thanh toán)
-- =======================================================================
INSERT INTO BillingSystem (transaction_id, account_id, method, payment_date) VALUES
(1001, 4, 'Transfer', '2025-10-20 10:30:00'), -- Giao dịch cho Invoice 2 (SV Vu)
(1002, 3, 'Cash', '2025-10-25 15:45:00');    -- Giao dịch cho Invoice 3 (SV Huong)

-- =======================================================================
-- IX. Dữ Liệu cho Bảng Invoice_Billing (Bảng trung gian)
-- =======================================================================
INSERT INTO Invoice_Billing (invoice_id, transaction_id) VALUES
(2, 1001), -- Liên kết Invoice 2 với Transaction 1001
(3, 1002); -- Liên kết Invoice 3 với Transaction 1002

-- =======================================================================
-- X. Dữ Liệu cho Bảng MaintenanceRequest (Yêu cầu Bảo trì)
-- =======================================================================
INSERT INTO MaintenanceRequest (request_id, description, date, status, student_id, manager_id, equip_id) VALUES
(1, 'Đèn trần phòng bị hỏng, cần thay thế.', '2025-11-18', 'Submitted', 3, NULL, 'LIGHT-1A'), -- Chưa có quản lý nhận
(2, 'Vòi nước nhà tắm rò rỉ.', '2025-11-15', 'In Progress', 4, 2, 'FAUCET-2B'); -- Đang được quản lý 2 xử lý

-- =======================================================================
-- XI. Dữ Liệu cho Bảng SystemLog (Nhật ký Hệ thống)
-- =======================================================================
INSERT INTO SystemLog (log_id, user_id, activity, datetime) VALUES
(1, 1, 'Admin logged in', '2025-11-20 08:00:00'),
(2, 2, 'Manager updated room A101 status', '2025-11-20 08:30:00'),
(3, 3, 'Student requested maintenance for room A101', '2025-11-20 09:00:00');

-- =======================================================================
-- XII. Dữ Liệu cho Bảng BackupResto (Sao lưu/Khôi phục)
-- =======================================================================
INSERT INTO BackupResto (backup_id, date, file_path, type, admin_id) VALUES
(1, '2025-11-10', '/backups/full_20251110.sql', 'Full', 1);