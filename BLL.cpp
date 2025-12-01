#define _CRT_SECURE_NO_WARNINGS
#include "BLL.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

using namespace std;

// Hàm trợ giúp để tạo SystemLog
void createSystemLog(DataAccessLayer& dal, int userId, const string& activity) {
    SystemLog log;
    log.log_id = 0;
    log.user_id = userId;
    log.activity = activity;

    // Lấy thời gian hiện tại và định dạng YYYY-MM-DD hh:mm:ss
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    log.datetime = buf;

    dal.addLog(log);
}

// -------------------------------------------------------------------
// I. CHỨC NĂNG TÀI KHOẢN VÀ NGƯỜI DÙNG
// -------------------------------------------------------------------

// UC1: Đăng ký một tài khoản người dùng mới vào hệ thống.
bool BusinessLogicLayer::registerAccount(const UserAccount& newUser) {
    if (newUser.username.empty() || newUser.password.empty()) {
        cerr << "Lỗi: Tên người dùng hoặc mật khẩu không được trống." << endl;
        return false;
    }


    if (dal.addUser(newUser)) {
        createSystemLog(dal, 0, "Đăng ký tài khoản mới: " + newUser.username + " với vai trò " + newUser.role);
        return true;
    }
    return false;
}

// UC2: Thực hiện đăng nhập cho người dùng.
bool BusinessLogicLayer::login(const string& username, const string& password, UserAccount& outUser) {

    vector<UserAccount> users = dal.getAllUsers();
    for (const auto& user : users) {
        if (user.username == username) {
            if (user.password == password) {
                outUser = user;
                createSystemLog(dal, user.account_id, "Đăng nhập thành công: " + username);
                return true;
            }
            else {
                createSystemLog(dal, user.account_id, "Đăng nhập thất bại: Sai mật khẩu cho " + username);
                return false;
            }
        }
    }

    createSystemLog(dal, 0, "Đăng nhập thất bại: Tên người dùng không tồn tại: " + username);
    return false;
}

// UC3: Cập nhật thông tin cá nhân (ngoại trừ mật khẩu) của người dùng.
bool BusinessLogicLayer::updateProfile(const UserAccount& updatedUser) {
    UserAccount existingUser = dal.getUserByID(updatedUser.account_id);
    if (existingUser.account_id == 0) {
        cerr << "Lỗi: Tài khoản ID " << updatedUser.account_id << " không tồn tại." << endl;
        return false;
    }

    UserAccount userToUpdate = updatedUser;
    userToUpdate.password = existingUser.password;

    if (dal.updateUser(userToUpdate)) {
        createSystemLog(dal, updatedUser.account_id, "Cập nhật hồ sơ thành công cho: " + updatedUser.username);
        return true;
    }
    return false;
}

// UC4: Thay đổi mật khẩu cho một tài khoản người dùng.
bool BusinessLogicLayer::changePassword(int accountId, const string& newPassword) {

    UserAccount user = dal.getUserByID(accountId);
    if (user.account_id == 0) {
        cerr << "Lỗi: Tài khoản ID " << accountId << " không tồn tại." << endl;
        return false;
    }
    if (newPassword.length() < 6) {
        cerr << "Lỗi: Mật khẩu mới quá ngắn." << endl;
        return false;
    }

    user.password = newPassword;
    if (dal.updateUser(user)) {
        createSystemLog(dal, accountId, "Thay đổi mật khẩu thành công.");
        return true;
    }
    return false;
}

// -------------------------------------------------------------------
// II. CHỨC NĂNG QUẢN LÝ PHÒNG VÀ SINH VIÊN
// -------------------------------------------------------------------

// UC5: Tìm kiếm các phòng còn trống dựa trên loại phòng hoặc tòa nhà.
vector<Room> BusinessLogicLayer::findAvailableRooms(const string& type, const string& building) {
    vector<Room> allRooms = dal.getAllRooms();
    vector<Room> availableRooms;

    string lowerType = type;
    string lowerBuilding = building;
    transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
    transform(lowerBuilding.begin(), lowerBuilding.end(), lowerBuilding.begin(), ::tolower);

    for (const auto& room : allRooms) {
        if (room.status == "Available" || room.status == "available") {
            bool matchesType = lowerType.empty() || lowerType == "all" ||
                (room.type.find(type) != string::npos);

            bool matchesBuilding = lowerBuilding.empty() || lowerBuilding == "all" ||
                (room.building.find(building) != string::npos);

            if (matchesType && matchesBuilding) {
                availableRooms.push_back(room);
            }
        }
    }
    return availableRooms;
}

// UC6: Thêm một phòng mới vào hệ thống (Manager/Admin).
bool BusinessLogicLayer::addNewRoom(const Room& newRoom) {

    if (newRoom.room_number.empty()) {
        cerr << "Lỗi: Số phòng không được trống." << endl;
        return false;
    }

    if (dal.addRoom(newRoom)) {
        createSystemLog(dal, 0, "Thêm phòng mới thành công: " + newRoom.room_number);
        return true;
    }
    return false;
}

// UC7: Lấy thông tin chi tiết của một sinh viên theo ID.
bool BusinessLogicLayer::getStudentDetails(int studentId, Student& outStudent) {
    outStudent = dal.getStudentByID(studentId);

    if (outStudent.student_id != 0) {
        return true;
    }
    cerr << "Lỗi: Không tìm thấy sinh viên ID " << studentId << endl;
    return false;
}

// UC8: Cập nhật thông tin chi tiết của một sinh viên (Manager/Admin).
bool BusinessLogicLayer::updateStudentDetails(const Student& updatedStudent) {

    if (dal.getStudentByID(updatedStudent.student_id).student_id == 0) {
        cerr << "Lỗi: Sinh viên ID " << updatedStudent.student_id << " không tồn tại." << endl;
        return false;
    }

    if (dal.updateStudent(updatedStudent)) {
        createSystemLog(dal, 0, "Cập nhật thông tin sinh viên thành công: ID " + to_string(updatedStudent.student_id));
        return true;
    }
    return false;
}

// -------------------------------------------------------------------
// III. CHỨC NĂNG QUẢN LÝ HỢP ĐỒNG VÀ THANH TOÁN
// -------------------------------------------------------------------

// UC9: Đăng ký một hợp đồng thuê phòng mới (Student/Manager).
bool BusinessLogicLayer::createNewContract(const Contract& newContract) {
    if (dal.getStudentByID(newContract.student_id).student_id == 0) {
        cerr << "Lỗi: Sinh viên ID " << newContract.student_id << " không tồn tại." << endl;
        return false;
    }

    Room room = dal.getRoomByID(newContract.room_id);
    if (room.room_id == 0 || room.status != "Available") {
        cerr << "Lỗi: Phòng ID " << newContract.room_id << " không tồn tại hoặc không còn trống." << endl;
        return false;
    }

    Contract contractToSave = newContract;
    contractToSave.status = "Active";

    if (dal.addContract(contractToSave)) {
        room.status = "Occupied";
        dal.updateRoom(room);

        createSystemLog(dal, 0, "Tạo hợp đồng mới thành công: ID " + to_string(contractToSave.contract_id));
        return true;
    }
    return false;
}

// UC10: Gia hạn một hợp đồng thuê phòng hiện có.
bool BusinessLogicLayer::renewContract(int contractId, const string& newEndDate) {
    Contract contract = dal.getContractByID(contractId);

    if (contract.contract_id == 0) {
        cerr << "Lỗi: Hợp đồng ID " << contractId << " không tồn tại." << endl;
        return false;
    }

    contract.end_date = newEndDate;
    contract.status = "Active";

    if (dal.updateContract(contract)) {
        createSystemLog(dal, 0, "Gia hạn hợp đồng ID " + to_string(contractId) + " đến " + newEndDate);
        return true;
    }
    return false;
}

// UC11: Lấy danh sách các hóa đơn cần thanh toán của một sinh viên.
vector<Invoice> BusinessLogicLayer::getPendingInvoices(int studentId) {
    vector<Invoice> allInvoices = dal.getAllInvoices();
    vector<Contract> allContracts = dal.getAllContracts();
    vector<Invoice> pendingInvoices;

    // 1. Tìm các contract_id của sinh viên
    vector<int> studentContractIds;
    for (const auto& contract : allContracts) {
        if (contract.student_id == studentId && contract.status == "Active") {
            studentContractIds.push_back(contract.contract_id);
        }
    }

    if (studentContractIds.empty()) {
        cerr << "Thông báo: Sinh viên ID " << studentId << " không có hợp đồng hoạt động." << endl;
        return pendingInvoices;
    }

    // 2. Lọc hóa đơn có status là "Pending" và thuộc về contract_id của sinh viên
    for (const auto& invoice : allInvoices) {
        if (invoice.status == "Pending" || invoice.status == "pending") {
            bool isStudentInvoice = false;
            for (int contractId : studentContractIds) {
                if (invoice.contract_id == contractId) {
                    isStudentInvoice = true;
                    break;
                }
            }
            if (isStudentInvoice) {
                pendingInvoices.push_back(invoice);
            }
        }
    }

    return pendingInvoices;
}

// UC12: Thực hiện thanh toán một hóa đơn cụ thể.
bool BusinessLogicLayer::payInvoice(int invoiceId, const string& paymentMethod, int accountId) {
    Invoice invoice = dal.getInvoiceByID(invoiceId);

    if (invoice.invoice_id == 0 || invoice.status != "Pending") {
        cerr << "Lỗi: Hóa đơn ID " << invoiceId << " không tồn tại hoặc đã được thanh toán." << endl;
        return false;
    }

    // 1. Tạo bản ghi giao dịch (BillingSystem)
    BillingSystem newTransaction;
    newTransaction.transaction_id = 0;
    newTransaction.account_id = accountId;
    newTransaction.method = paymentMethod;
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    newTransaction.payment_date = buf;

    if (!dal.addTransaction(newTransaction)) {
        cerr << "Lỗi: Không thể thêm giao dịch." << endl;
        return false;
    }

    int lastTransactionId = 0;
    vector<BillingSystem> allTransactions = dal.getAllTransactions();
    if (!allTransactions.empty()) {
        lastTransactionId = allTransactions.back().transaction_id;
    }
    else {
        cerr << "Cảnh báo: Không thể lấy transaction_id." << endl;
    }

    // 2. Tạo bản ghi liên kết Invoice_Billing
    if (lastTransactionId != 0) {
        Invoice_Billing ib;
        ib.invoice_id = invoiceId;
        ib.transaction_id = lastTransactionId;
        if (!dal.addInvoiceBilling(ib)) {
            cerr << "Lỗi: Không thể liên kết hóa đơn và giao dịch." << endl;
            return false;
        }
    }

    // 3. Cập nhật trạng thái hóa đơn thành "Paid"
    invoice.status = "Paid";
    if (dal.updateInvoice(invoice)) {
        createSystemLog(dal, accountId, "Thanh toán hóa đơn ID " + to_string(invoiceId) + " thành công.");
        return true;
    }

    return false;
}

// UC13: Lấy danh sách các sinh viên sắp hết hạn hợp đồng trong X ngày tới (Manager).
vector<Student> BusinessLogicLayer::getStudentsWithExpiringContracts(int days) {

    vector<Contract> allContracts = dal.getAllContracts();
    vector<Student> expiringStudents;


    for (const auto& contract : allContracts) {
        if (contract.status == "Active") {
            Student student = dal.getStudentByID(contract.student_id);
            if (student.student_id != 0) {
                expiringStudents.push_back(student);
            }
        }
    }

    sort(expiringStudents.begin(), expiringStudents.end(), [](const Student& a, const Student& b) {
        return a.student_id < b.student_id;
        });
    expiringStudents.erase(unique(expiringStudents.begin(), expiringStudents.end(), [](const Student& a, const Student& b) {
        return a.student_id == b.student_id;
        }), expiringStudents.end());

    return expiringStudents;
}

// UC14: Lập hóa đơn định kỳ (điện, nước, internet) cho tất cả các hợp đồng đang hoạt động (Manager/Admin).
bool BusinessLogicLayer::generateMonthlyInvoices(int month, int year) {
    vector<Contract> activeContracts = dal.getAllContracts();
    int successCount = 0;

    double baseAmount = 500000.0;

    string dueDate = to_string(year) + "-" + (month < 10 ? "0" : "") + to_string(month) + "-15";

    for (const auto& contract : activeContracts) {
        if (contract.status == "Active") {
            Invoice newInvoice;
            newInvoice.invoice_id = 0;
            newInvoice.contract_id = contract.contract_id;
            newInvoice.amount = baseAmount;
            newInvoice.due_date = dueDate;
            newInvoice.status = "Pending";

            if (dal.addInvoice(newInvoice)) {
                successCount++;
            }
        }
    }

    if (successCount > 0) {
        createSystemLog(dal, 0, "Lập " + to_string(successCount) + " hóa đơn tháng " + to_string(month) + "/" + to_string(year) + " thành công.");
        return true;
    }
    return false;
}

// -------------------------------------------------------------------
// IV. CHỨC NĂNG BẢO TRÌ VÀ HỆ THỐNG
// -------------------------------------------------------------------

// UC15: Gửi yêu cầu bảo trì/sửa chữa thiết bị (Student/Manager).
bool BusinessLogicLayer::submitMaintenanceRequest(const MaintenanceRequest& newRequest) {
    MaintenanceRequest requestToSave = newRequest;
    requestToSave.request_id = 0;
    requestToSave.status = "Submitted";

    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    requestToSave.date = buf;

    if (dal.addMaintenanceRequest(requestToSave)) {
        createSystemLog(dal, requestToSave.student_id, "Gửi yêu cầu bảo trì: " + requestToSave.description);
        return true;
    }
    return false;
}

// UC16: Cập nhật trạng thái yêu cầu bảo trì (Manager).
bool BusinessLogicLayer::updateRequestStatus(int requestId, const string& newStatus, int managerId) {
    MaintenanceRequest request = dal.getMaintenanceRequestByID(requestId);

    if (request.request_id == 0) {
        cerr << "Lỗi: Yêu cầu bảo trì ID " << requestId << " không tồn tại." << endl;
        return false;
    }
    if (newStatus != "In Progress" && newStatus != "Completed" && newStatus != "Canceled") {
        cerr << "Lỗi: Trạng thái không hợp lệ." << endl;
        return false;
    }

    request.status = newStatus;
    request.manager_id = managerId;

    if (dal.updateMaintenanceRequest(request)) {
        createSystemLog(dal, 0, "Cập nhật trạng thái yêu cầu ID " + to_string(requestId) + " thành " + newStatus + " bởi Manager ID " + to_string(managerId));
        return true;
    }
    return false;
}

// UC17: Xem nhật ký hoạt động của hệ thống theo ngày (Admin).
vector<SystemLog> BusinessLogicLayer::viewSystemLogsByDate(const string& date) {
    vector<SystemLog> allLogs = dal.getAllLogs();
    vector<SystemLog> filteredLogs;

    for (const auto& log : allLogs) {
        if (log.datetime.rfind(date, 0) == 0) {
            filteredLogs.push_back(log);
        }
    }
    return filteredLogs;
}

// UC18: Lấy báo cáo thống kê doanh thu theo tháng/năm (Admin).
double BusinessLogicLayer::getRevenueReport(int month, int year) {
    vector<Invoice> allInvoices = dal.getAllInvoices();
    vector<Invoice_Billing> allInvoiceBillings = dal.getAllInvoiceBilling();
    vector<BillingSystem> allTransactions = dal.getAllTransactions();
    double totalRevenue = 0.0;

    string monthStr = (month < 10 ? "0" : "") + to_string(month);
    string yearStr = to_string(year);
    string datePrefix = yearStr + "-" + monthStr;

    // 1. Lấy tất cả Transaction_ID được thanh toán trong tháng/năm
    vector<int> targetTransactionIds;
    for (const auto& transaction : allTransactions) {
        if (transaction.payment_date.rfind(datePrefix, 0) == 0) {
            targetTransactionIds.push_back(transaction.transaction_id);
        }
    }

    // 2. Lấy Invoice_ID từ các Transaction_ID đó
    vector<int> targetInvoiceIds;
    for (const auto& ib : allInvoiceBillings) {
        bool isTarget = false;
        for (int tid : targetTransactionIds) {
            if (ib.transaction_id == tid) {
                targetInvoiceIds.push_back(ib.invoice_id);
                break;
            }
        }
    }

    // 3. Tính tổng amount từ các Invoice_ID này
    for (const auto& invoice : allInvoices) {
        bool isTarget = false;
        for (int iid : targetInvoiceIds) {
            if (invoice.invoice_id == iid) {
                totalRevenue += invoice.amount;
                break;
            }
        }
    }

    return totalRevenue;
}

// UC19: Thực hiện sao lưu cơ sở dữ liệu (Admin).
bool BusinessLogicLayer::performDatabaseBackup(const string& type, int adminId) {

    BackupResto newBackup;
    newBackup.backup_id = 0;

    time_t now = time(0);
    struct tm tstruct;
    char bufDate[80];
    tstruct = *localtime(&now);
    strftime(bufDate, sizeof(bufDate), "%Y-%m-%d", &tstruct);
    newBackup.date = bufDate;

    string filePath = "/backups/db_backup_" + newBackup.date + "_" + type + ".sql";
    newBackup.file_path = filePath;
    newBackup.type = type;
    newBackup.admin_id = adminId;

    if (dal.addBackup(newBackup)) {
        createSystemLog(dal, adminId, "Sao lưu cơ sở dữ liệu loại " + type + " thành công.");
        return true;
    }
    return false;

}
// --- TRIỂN KHAI HÀM GHI LOG (SỬA LỖI MAIN.CPP) ---
bool BusinessLogicLayer::addLog(int userId, const string& activity) {
    SystemLog log;
    log.log_id = 0;
    log.user_id = userId;
    log.activity = activity;

    // Lấy thời gian hiện tại
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    log.datetime = buf;

    return dal.addLog(log);
}