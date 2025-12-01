#pragma once
#ifndef BUSINESSLOGICLAYER_H
#define BUSINESSLOGICLAYER_H
#include "Struct.h"          
#include "DataAccessLayer.h" 

class BusinessLogicLayer
{
private:
    DataAccessLayer dal; // Đối tượng truy cập dữ liệu

public:
    // Constructor 
    BusinessLogicLayer() = default;
    // -------------------------------------------------------------------
    // I. CHỨC NĂNG TÀI KHOẢN VÀ NGƯỜI DÙNG (Use Case 1 - 4)
    // -------------------------------------------------------------------

    // @brief UC1: Đăng ký một tài khoản người dùng mới vào hệ thống.

    bool registerAccount(const UserAccount& newUser);

    //@brief UC2: Thực hiện đăng nhập cho người dùng.

    bool login(const std::string& username, const std::string& password, UserAccount& outUser);

    // @brief UC3: Cập nhật thông tin cá nhân (ngoại trừ mật khẩu) của người dùng.

    bool updateProfile(const UserAccount& updatedUser);

    //@brief UC4: Thay đổi mật khẩu cho một tài khoản người dùng.

    bool changePassword(int accountId, const std::string& newPassword);


    // -------------------------------------------------------------------
    // II. CHỨC NĂNG QUẢN LÝ PHÒNG VÀ SINH VIÊN (Use Case 5 - 8)
    // -------------------------------------------------------------------

    // @brief UC5: Tìm kiếm các phòng còn trống dựa trên loại phòng hoặc tòa nhà.

    std::vector<Room> findAvailableRooms(const std::string& type, const std::string& building);

    // @brief UC6: Thêm một phòng mới vào hệ thống (Manager/Admin).

    bool addNewRoom(const Room& newRoom);

    // @brief UC7: Lấy thông tin chi tiết của một sinh viên theo ID.

    bool getStudentDetails(int studentId, Student& outStudent);

    // @brief UC8: Cập nhật thông tin chi tiết của một sinh viên (Manager/Admin).

    bool updateStudentDetails(const Student& updatedStudent);


    // -------------------------------------------------------------------
    // III. CHỨC NĂNG QUẢN LÝ HỢP ĐỒNG VÀ THANH TOÁN (Use Case 9 - 14)
    // -------------------------------------------------------------------

    // @brief UC9: Đăng ký một hợp đồng thuê phòng mới (Student/Manager).

    bool createNewContract(const Contract& newContract);

    // @brief UC10: Gia hạn một hợp đồng thuê phòng hiện có.

    bool renewContract(int contractId, const std::string& newEndDate);

    // @brief UC11: Lấy danh sách các hóa đơn cần thanh toán của một sinh viên.

    std::vector<Invoice> getPendingInvoices(int studentId);

    // @brief UC12: Thực hiện thanh toán một hóa đơn cụ thể.

    bool payInvoice(int invoiceId, const std::string& paymentMethod, int accountId);

    // @brief UC13: Lấy danh sách các sinh viên sắp hết hạn hợp đồng trong X ngày tới (Manager).

    std::vector<Student> getStudentsWithExpiringContracts(int days);

    // @brief UC14: Lập hóa đơn định kỳ (điện, nước, internet) cho tất cả các hợp đồng đang hoạt động (Manager/Admin).

    bool generateMonthlyInvoices(int month, int year);


    // -------------------------------------------------------------------
    // IV. CHỨC NĂNG BẢO TRÌ VÀ HỆ THỐNG (Use Case 15 - 19)
    // -------------------------------------------------------------------

    // @brief UC15: Gửi yêu cầu bảo trì/sửa chữa thiết bị (Student/Manager).

    bool submitMaintenanceRequest(const MaintenanceRequest& newRequest);

    // @brief UC16: Cập nhật trạng thái yêu cầu bảo trì (Manager).

    bool updateRequestStatus(int requestId, const std::string& newStatus, int managerId);

    //@brief UC17: Xem nhật ký hoạt động của hệ thống theo ngày (Admin).

    std::vector<SystemLog> viewSystemLogsByDate(const std::string& date);

    // @brief UC18: Lấy báo cáo thống kê doanh thu theo tháng/năm (Admin).

    double getRevenueReport(int month, int year);

    //@brief UC19: Thực hiện sao lưu cơ sở dữ liệu (Admin).

    bool performDatabaseBackup(const std::string& type, int adminId);

    // --- HÀM HỖ TRỢ GHI LOG (MỚI THÊM ĐỂ SỬA LỖI) ---
    // @brief Ghi lại hoạt động hệ thống (được gọi từ Main.cpp)
    bool addLog(int userId, const std::string& activity);


};

#endif // BUSINESSLOGICLAYER_H