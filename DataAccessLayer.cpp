#include "Struct.h"
#include "DataAccessLayer.h"
#include <iostream>
#include <windows.h>
#include <sqlext.h>
#include <vector>

// Macro kích thước chuỗi an toàn
#define MAX_DATA 256

using namespace std;

// ======================= KẾT NỐI & TIỆN ÍCH =======================

DataAccessLayer::DataAccessLayer() {
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hConn);
}

DataAccessLayer::~DataAccessLayer() {
    disconnect();
    SQLFreeHandle(SQL_HANDLE_DBC, hConn);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

void DataAccessLayer::showError(unsigned int handleType, const SQLHANDLE& handle) {
    SQLCHAR SQLState[1024];
    SQLCHAR message[1024];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;
    if (SQL_SUCCESS == SQLGetDiagRecA(handleType, handle, 1, SQLState, &nativeError, message, sizeof(message), &textLength)) {
        cerr << "SQL ERROR: " << message << endl;
    }
}

bool DataAccessLayer::connect() {
    // LƯU Ý: Hãy chắc chắn DSN 'tbi' đã được cấu hình đúng trong ODBC Data Sources (32-bit hoặc 64-bit tùy project)
    string connStr = "DSN=tbi;UID=root;PWD=1234;DATABASE=quan_ly_ky_tuc_xa;";
    SQLRETURN ret = SQLDriverConnectA(hConn, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    if (!SQL_SUCCEEDED(ret)) {
        showError(SQL_HANDLE_DBC, hConn);
        return false;
    }
    return true;
}

void DataAccessLayer::disconnect() {
    SQLDisconnect(hConn);
}

// ======================= ROOM CRUD (FIXED) =======================

bool DataAccessLayer::addRoom(const Room& r) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "INSERT INTO Room (room_number, type, capacity, status, building) VALUES (?, ?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER cap = r.capacity;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.room_number.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.type.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cap, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.building.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::updateRoom(const Room& r) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE Room SET room_number=?, type=?, capacity=?, status=?, building=? WHERE room_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER cap = r.capacity;
    SQLINTEGER rid = r.room_id;

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.room_number.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.type.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cap, 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)r.building.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::deleteRoom(int room_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "DELETE FROM Room WHERE room_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &room_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

Room DataAccessLayer::getRoomByID(int room_id) {
    Room r; // Đã được khởi tạo default = 0 trong Struct.h
    if (!connect()) return r;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT room_id, room_number, type, capacity, status, building FROM Room WHERE room_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &room_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        // FIX C6054: Khởi tạo buffer bằng {0}
        char num[50] = { 0 }, typ[50] = { 0 }, sta[50] = { 0 }, bui[50] = { 0 };
        SQLINTEGER cap, rid;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &rid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, num, sizeof(num), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, typ, sizeof(typ), NULL);
        SQLGetData(hStmt, 4, SQL_C_SLONG, &cap, 0, NULL);
        SQLGetData(hStmt, 5, SQL_C_CHAR, sta, sizeof(sta), NULL);
        SQLGetData(hStmt, 6, SQL_C_CHAR, bui, sizeof(bui), NULL);

        r.room_id = rid; r.room_number = num; r.type = typ; r.capacity = cap; r.status = sta; r.building = bui;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return r;
}

std::vector<Room> DataAccessLayer::getAllRooms() {
    std::vector<Room> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT room_id, room_number, type, capacity, status, building FROM Room", SQL_NTS);

    // FIX C6054: Khởi tạo buffer bằng {0}
    char num[50] = { 0 }, typ[50] = { 0 }, sta[50] = { 0 }, bui[50] = { 0 };
    SQLINTEGER cap = 0, rid = 0;

    // Bind cột để lấy dữ liệu nhanh hơn
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &rid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, num, sizeof(num), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, typ, sizeof(typ), NULL);
    SQLBindCol(hStmt, 4, SQL_C_SLONG, &cap, 0, NULL);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, sta, sizeof(sta), NULL);
    SQLBindCol(hStmt, 6, SQL_C_CHAR, bui, sizeof(bui), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Room r;
        r.room_id = rid; r.room_number = num; r.type = typ; r.capacity = cap; r.status = sta; r.building = bui;
        list.push_back(r);
        // Reset buffer cho vòng lặp tiếp theo (an toàn)
        memset(num, 0, sizeof(num)); memset(typ, 0, sizeof(typ));
        memset(sta, 0, sizeof(sta)); memset(bui, 0, sizeof(bui));
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= STUDENT CRUD (FIXED) =======================

bool DataAccessLayer::addStudent(const Student& s) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "INSERT INTO Student (name, email, phone, gender, DOB, address) VALUES (?, ?, ?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)s.name.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)s.email.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.phone.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.gender.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.DOB.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 150, 0, (SQLPOINTER)s.address.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::updateStudent(const Student& s) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE Student SET name=?, email=?, phone=?, gender=?, DOB=?, address=? WHERE student_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER sid = s.student_id;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)s.name.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)s.email.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.phone.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.gender.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)s.DOB.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 150, 0, (SQLPOINTER)s.address.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::deleteStudent(int student_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM Student WHERE student_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &student_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

Student DataAccessLayer::getStudentByID(int student_id) {
    Student s;
    if (!connect()) return s;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT student_id, name, email, phone, gender, DOB, address FROM Student WHERE student_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &student_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        // FIX C6054: Khởi tạo buffer bằng {0}
        char nam[100] = { 0 }, ema[100] = { 0 }, pho[20] = { 0 }, gen[20] = { 0 }, dob[20] = { 0 }, add[150] = { 0 };
        SQLINTEGER sid;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &sid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, nam, sizeof(nam), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ema, sizeof(ema), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, pho, sizeof(pho), NULL);
        SQLGetData(hStmt, 5, SQL_C_CHAR, gen, sizeof(gen), NULL);
        SQLGetData(hStmt, 6, SQL_C_CHAR, dob, sizeof(dob), NULL);
        SQLGetData(hStmt, 7, SQL_C_CHAR, add, sizeof(add), NULL);

        s.student_id = sid; s.name = nam; s.email = ema; s.phone = pho; s.gender = gen; s.DOB = dob; s.address = add;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return s;
}

std::vector<Student> DataAccessLayer::getAllStudents() {
    std::vector<Student> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT student_id, name, email, phone, gender, DOB, address FROM Student", SQL_NTS);

    // FIX C6054: Khởi tạo buffer bằng {0}
    char nam[100] = { 0 }, ema[100] = { 0 }, pho[20] = { 0 }, gen[20] = { 0 }, dob[20] = { 0 }, add[150] = { 0 };
    SQLINTEGER sid = 0;

    SQLBindCol(hStmt, 1, SQL_C_SLONG, &sid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, nam, sizeof(nam), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, ema, sizeof(ema), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, pho, sizeof(pho), NULL);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, gen, sizeof(gen), NULL);
    SQLBindCol(hStmt, 6, SQL_C_CHAR, dob, sizeof(dob), NULL);
    SQLBindCol(hStmt, 7, SQL_C_CHAR, add, sizeof(add), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Student s;
        s.student_id = sid; s.name = nam; s.email = ema; s.phone = pho; s.gender = gen; s.DOB = dob; s.address = add;
        list.push_back(s);
        // Reset buffer cho an toàn
        memset(nam, 0, sizeof(nam)); memset(ema, 0, sizeof(ema));
        memset(pho, 0, sizeof(pho)); memset(gen, 0, sizeof(gen));
        memset(dob, 0, sizeof(dob)); memset(add, 0, sizeof(add));
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= MANAGER CRUD (FIXED) =======================
//add manager
bool DataAccessLayer::addManager(const DormitoryManager& m) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "INSERT INTO DormitoryManager (manager_id, name, email, phone) VALUES (?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER mid = m.manager_id;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &mid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)m.name.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)m.email.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)m.phone.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
// UPDATE MANAGER
bool DataAccessLayer::updateManager(const DormitoryManager& m) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE DormitoryManager SET name=?, email=?, phone=? WHERE manager_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER mid = m.manager_id;
    // Bind tham số
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)m.name.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLPOINTER)m.email.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLPOINTER)m.phone.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &mid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// DELETE MANAGER
bool DataAccessLayer::deleteManager(int manager_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "DELETE FROM DormitoryManager WHERE manager_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &manager_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// GET MANAGER BY ID
DormitoryManager DataAccessLayer::getManagerByID(int manager_id) {
    DormitoryManager m; // Đã được khởi tạo = 0 nhờ Struct.h fix
    if (!connect()) return m;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT manager_id, name, email, phone FROM DormitoryManager WHERE manager_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &manager_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        char nam[100] = { 0 }, ema[100] = { 0 }, pho[20] = { 0 }; // Buffer khởi tạo {0} sửa lỗi C6054
        SQLINTEGER mid;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &mid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, nam, sizeof(nam), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, ema, sizeof(ema), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, pho, sizeof(pho), NULL);

        m.manager_id = mid;
        m.name = nam;
        m.email = ema;
        m.phone = pho;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return m;
}

// GET ALL MANAGERS
std::vector<DormitoryManager> DataAccessLayer::getAllManagers() {
    std::vector<DormitoryManager> list;
    if (!connect()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT manager_id, name, email, phone FROM DormitoryManager", SQL_NTS);

    char nam[100] = { 0 }, ema[100] = { 0 }, pho[20] = { 0 };
    SQLINTEGER mid = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &mid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, nam, sizeof(nam), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, ema, sizeof(ema), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, pho, sizeof(pho), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        DormitoryManager m;
        m.manager_id = mid;
        m.name = nam;
        m.email = ema;
        m.phone = pho;
        list.push_back(m);

        // Reset buffer
        memset(nam, 0, sizeof(nam));
        memset(ema, 0, sizeof(ema));
        memset(pho, 0, sizeof(pho));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= CONTRACT CRUD (FIXED: STATUS) =======================

bool DataAccessLayer::addContract(const Contract& c) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // FIX: Dùng cột status, dùng dấu ? để bind tham số
    const char* sql = "INSERT INTO Contract (student_id, room_id, start_date, end_date, status) VALUES (?, ?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER sid = c.student_id, rid = c.room_id;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rid, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.start_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.end_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.status.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::updateContract(const Contract& c) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE Contract SET student_id=?, room_id=?, start_date=?, end_date=?, status=? WHERE contract_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER sid = c.student_id, rid = c.room_id, cid = c.contract_id;
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rid, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.start_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.end_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)c.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &cid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

bool DataAccessLayer::deleteContract(int contract_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM Contract WHERE contract_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &contract_id, 0, NULL);
    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

Contract DataAccessLayer::getContractByID(int contract_id) {
    Contract c;
    if (!connect()) return c;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT contract_id, student_id, room_id, start_date, end_date, status FROM Contract WHERE contract_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &contract_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        // FIX C6054: Khởi tạo buffer bằng {0}
        char sd[50] = { 0 }, ed[50] = { 0 }, sta[50] = { 0 };
        SQLINTEGER cid, sid, rid;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &cid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_SLONG, &sid, 0, NULL);
        SQLGetData(hStmt, 3, SQL_C_SLONG, &rid, 0, NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, sd, sizeof(sd), NULL);
        SQLGetData(hStmt, 5, SQL_C_CHAR, ed, sizeof(ed), NULL);
        SQLGetData(hStmt, 6, SQL_C_CHAR, sta, sizeof(sta), NULL);

        c.contract_id = cid; c.student_id = sid; c.room_id = rid; c.start_date = sd; c.end_date = ed; c.status = sta;
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return c;
}

std::vector<Contract> DataAccessLayer::getAllContracts() {
    std::vector<Contract> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT contract_id, student_id, room_id, start_date, end_date, status FROM Contract", SQL_NTS);

    // FIX C6054: Khởi tạo buffer bằng {0}
    char sd[50] = { 0 }, ed[50] = { 0 }, sta[50] = { 0 };
    SQLINTEGER cid = 0, sid = 0, rid = 0;

    SQLBindCol(hStmt, 1, SQL_C_SLONG, &cid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, &sid, 0, NULL);
    SQLBindCol(hStmt, 3, SQL_C_SLONG, &rid, 0, NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, sd, sizeof(sd), NULL);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, ed, sizeof(ed), NULL);
    SQLBindCol(hStmt, 6, SQL_C_CHAR, sta, sizeof(sta), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Contract c;
        c.contract_id = cid; c.student_id = sid; c.room_id = rid; c.start_date = sd; c.end_date = ed; c.status = sta;
        list.push_back(c);
        // Reset buffer
        memset(sd, 0, sizeof(sd)); memset(ed, 0, sizeof(ed)); memset(sta, 0, sizeof(sta));
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}
// ======================= INVOICE CRUD (FIXED) =======================

// ADD INVOICE
bool DataAccessLayer::addInvoice(const Invoice& i) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "INSERT INTO Invoice (contract_id, amount, due_date, status) VALUES (?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Khai báo biến trung gian để bind (đảm bảo địa chỉ bộ nhớ hợp lệ)
    SQLINTEGER contract_id = i.contract_id;
    SQLDOUBLE amount = i.amount;

    // Bind tham số
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &contract_id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &amount, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)i.due_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)i.status.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// UPDATE INVOICE
bool DataAccessLayer::updateInvoice(const Invoice& i) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE Invoice SET contract_id=?, amount=?, due_date=?, status=? WHERE invoice_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER contract_id = i.contract_id;
    SQLDOUBLE amount = i.amount;
    SQLINTEGER invoice_id = i.invoice_id;

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &contract_id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &amount, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)i.due_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)i.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoice_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// DELETE INVOICE
bool DataAccessLayer::deleteInvoice(int invoice_id) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "DELETE FROM Invoice WHERE invoice_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoice_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// GET INVOICE BY ID
Invoice DataAccessLayer::getInvoiceByID(int invoice_id) {
    Invoice i; // Đã được khởi tạo default = 0/0.0 trong Struct.h
    if (!connect()) return i;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT invoice_id, contract_id, amount, due_date, status FROM Invoice WHERE invoice_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoice_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        // Khởi tạo buffer chuỗi {0} để tránh lỗi C6054
        char due[50] = { 0 }, stat[50] = { 0 };
        SQLINTEGER inv_id, con_id;
        SQLDOUBLE amt;

        // Lấy dữ liệu từng cột
        SQLGetData(hStmt, 1, SQL_C_SLONG, &inv_id, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_SLONG, &con_id, 0, NULL);
        SQLGetData(hStmt, 3, SQL_C_DOUBLE, &amt, 0, NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, due, sizeof(due), NULL);
        SQLGetData(hStmt, 5, SQL_C_CHAR, stat, sizeof(stat), NULL);

        // Gán vào Struct
        i.invoice_id = inv_id;
        i.contract_id = con_id;
        i.amount = amt;
        i.due_date = due;
        i.status = stat;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return i;
}

// GET ALL INVOICES
std::vector<Invoice> DataAccessLayer::getAllInvoices() {
    std::vector<Invoice> list;
    if (!connect()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT invoice_id, contract_id, amount, due_date, status FROM Invoice", SQL_NTS);

    char due[50] = { 0 }, stat[50] = { 0 };
    SQLINTEGER inv_id = 0, con_id = 0;
    SQLDOUBLE amt = 0.0;

    // Bind cột để fetch nhanh hơn (tùy chọn, dùng SQLGetData như trên cũng được)
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &inv_id, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, &con_id, 0, NULL);
    SQLBindCol(hStmt, 3, SQL_C_DOUBLE, &amt, 0, NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, due, sizeof(due), NULL);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, stat, sizeof(stat), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Invoice i;
        i.invoice_id = inv_id;
        i.contract_id = con_id;
        i.amount = amt;
        i.due_date = due;
        i.status = stat;
        list.push_back(i);

        // Reset buffer (cực kỳ quan trọng để tránh dữ liệu cũ còn sót lại)
        memset(due, 0, sizeof(due));
        memset(stat, 0, sizeof(stat));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}
// ======================= MAINTENANCE REQUEST CRUD (FIXED) =======================

// ADD MAINTENANCE REQUEST
bool DataAccessLayer::addMaintenanceRequest(const MaintenanceRequest& mr) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "INSERT INTO MaintenanceRequest (description, date, status, student_id, manager_id, equip_id) VALUES (?, ?, ?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Biến trung gian để bind
    SQLINTEGER sid = mr.student_id;
    SQLINTEGER mid = mr.manager_id;

    // Bind tham số
    // Description có thể dài, ta cho max 255 hoặc tùy chỉnh theo CSDL
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)mr.description.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sid, 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &mid, 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.equip_id.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// UPDATE MAINTENANCE REQUEST
bool DataAccessLayer::updateMaintenanceRequest(const MaintenanceRequest& mr) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "UPDATE MaintenanceRequest SET description=?, date=?, status=?, student_id=?, manager_id=?, equip_id=? WHERE request_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER sid = mr.student_id;
    SQLINTEGER mid = mr.manager_id;
    SQLINTEGER rid = mr.request_id;

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)mr.description.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.status.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sid, 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &mid, 0, NULL);
    SQLBindParameter(hStmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)mr.equip_id.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 7, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// DELETE MAINTENANCE REQUEST
bool DataAccessLayer::deleteMaintenanceRequest(int request_id) {
    if (!connect()) return false;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "DELETE FROM MaintenanceRequest WHERE request_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &request_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// GET MAINTENANCE REQUEST BY ID
MaintenanceRequest DataAccessLayer::getMaintenanceRequestByID(int request_id) {
    MaintenanceRequest mr; // Đã được khởi tạo default nhờ Struct.h
    if (!connect()) return mr;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    const char* sql = "SELECT request_id, description, date, status, student_id, manager_id, equip_id FROM MaintenanceRequest WHERE request_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &request_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        // Khởi tạo buffer {0} để tránh lỗi C6054
        char desc[256] = { 0 }; // Description có thể dài
        char date[50] = { 0 };
        char stat[50] = { 0 };
        char equip[50] = { 0 };
        SQLINTEGER rid, sid, mid;

        // Lấy dữ liệu
        SQLGetData(hStmt, 1, SQL_C_SLONG, &rid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, desc, sizeof(desc), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, date, sizeof(date), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, stat, sizeof(stat), NULL);
        SQLGetData(hStmt, 5, SQL_C_SLONG, &sid, 0, NULL);
        SQLGetData(hStmt, 6, SQL_C_SLONG, &mid, 0, NULL);
        SQLGetData(hStmt, 7, SQL_C_CHAR, equip, sizeof(equip), NULL);

        // Gán vào Struct
        mr.request_id = rid;
        mr.description = desc;
        mr.date = date;
        mr.status = stat;
        mr.student_id = sid;
        mr.manager_id = mid;
        mr.equip_id = equip;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return mr;
}

// GET ALL MAINTENANCE REQUESTS
std::vector<MaintenanceRequest> DataAccessLayer::getAllMaintenanceRequests() {
    std::vector<MaintenanceRequest> list;
    if (!connect()) return list;

    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT request_id, description, date, status, student_id, manager_id, equip_id FROM MaintenanceRequest", SQL_NTS);

    // Buffer
    char desc[256] = { 0 };
    char date[50] = { 0 };
    char stat[50] = { 0 };
    char equip[50] = { 0 };
    SQLINTEGER rid = 0, sid = 0, mid = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &rid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, desc, sizeof(desc), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, date, sizeof(date), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, stat, sizeof(stat), NULL);
    SQLBindCol(hStmt, 5, SQL_C_SLONG, &sid, 0, NULL);
    SQLBindCol(hStmt, 6, SQL_C_SLONG, &mid, 0, NULL);
    SQLBindCol(hStmt, 7, SQL_C_CHAR, equip, sizeof(equip), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        MaintenanceRequest mr;
        mr.request_id = rid;
        mr.description = desc;
        mr.date = date;
        mr.status = stat;
        mr.student_id = sid;
        mr.manager_id = mid;
        mr.equip_id = equip;

        list.push_back(mr);

        // Reset buffer
        memset(desc, 0, sizeof(desc));
        memset(date, 0, sizeof(date));
        memset(stat, 0, sizeof(stat));
        memset(equip, 0, sizeof(equip));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

//====================== = USER CRUD(FIXED) ====================== =
//adduser
bool DataAccessLayer::addUser(const UserAccount& u) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Đã sửa: Thay đổi từ password_hash thành password
    const char* sql = "INSERT INTO UserAccount (username, password, role) VALUES (?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (username, password, role)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)u.username.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)u.password.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)u.role.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//updateuser
bool DataAccessLayer::updateUser(const UserAccount& u) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Đã sửa: Thay đổi từ password_hash thành password
    const char* sql = "UPDATE UserAccount SET username=?, password=?, role=? WHERE account_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER acc_id = u.account_id;

    // Bind tham số (username, password, role, account_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)u.username.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)u.password.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)u.role.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &acc_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//delete user
bool DataAccessLayer::deleteUser(int account_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM UserAccount WHERE account_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (account_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &account_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//get user by id
UserAccount DataAccessLayer::getUserByID(int account_id) {
    UserAccount u;
    if (!connect()) return u;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Đã sửa: Thay đổi từ password_hash thành password
    const char* sql = "SELECT account_id, username, password, role FROM UserAccount WHERE account_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &account_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        char user[256] = { 0 }, pass[256] = { 0 }, role[50] = { 0 }; // pass cho password
        SQLINTEGER acc_id = 0;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &acc_id, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, user, sizeof(user), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, pass, sizeof(pass), NULL); // Lấy password
        SQLGetData(hStmt, 4, SQL_C_CHAR, role, sizeof(role), NULL);

        u.account_id = acc_id;
        u.username = user;
        u.password = pass; // Lưu vào trường password
        u.role = role;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return u;
}
//getalluser
std::vector<UserAccount> DataAccessLayer::getAllUsers() {
    std::vector<UserAccount> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Đã sửa: Thay đổi từ password_hash thành password
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT account_id, username, password, role FROM UserAccount", SQL_NTS);

    char user[256] = { 0 }, pass[256] = { 0 }, role[50] = { 0 }; // pass cho password
    SQLINTEGER acc_id = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &acc_id, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, user, sizeof(user), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, pass, sizeof(pass), NULL); // Bind password
    SQLBindCol(hStmt, 4, SQL_C_CHAR, role, sizeof(role), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        UserAccount u;
        u.account_id = acc_id;
        u.username = user;
        u.password = pass; // Lưu vào trường password
        u.role = role;
        list.push_back(u);

        // Reset buffer
        memset(user, 0, sizeof(user));
        memset(pass, 0, sizeof(pass));
        memset(role, 0, sizeof(role));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

//================ ADMIN ========================
//addadmin
bool DataAccessLayer::addAdmin(const Admin& a) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // Thêm Admin vào bảng. admin_id được cung cấp (từ UserAccount)
    const char* sql = "INSERT INTO Admin (admin_id, username, password, privilege) VALUES (?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER admin_id = a.admin_id;

    // Bind tham số (admin_id, username, password, privilege)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &admin_id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)a.username.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)a.password.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)a.privilege.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//update admin
bool DataAccessLayer::updateAdmin(const Admin& a) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Cập nhật các trường: username, password, privilege dựa trên admin_id
    const char* sql = "UPDATE Admin SET username=?, password=?, privilege=? WHERE admin_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER admin_id = a.admin_id;

    // Bind tham số (username, password, privilege, admin_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)a.username.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)a.password.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)a.privilege.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &admin_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//delete admin
bool DataAccessLayer::deleteAdmin(int admin_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM Admin WHERE admin_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (admin_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &admin_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}
//get admin
Admin DataAccessLayer::getAdminByID(int admin_id) {
    Admin a;
    if (!connect()) return a;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "SELECT admin_id, username, password, privilege FROM Admin WHERE admin_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &admin_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        char user[50] = { 0 }, pass[256] = { 0 }, priv[50] = { 0 };
        SQLINTEGER aid = 0;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &aid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, user, sizeof(user), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, pass, sizeof(pass), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, priv, sizeof(priv), NULL);

        a.admin_id = aid;
        a.username = user;
        a.password = pass;
        a.privilege = priv;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return a;
}
// get all admin
std::vector<Admin> DataAccessLayer::getAllAdmins() {
    std::vector<Admin> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT admin_id, username, password, privilege FROM Admin", SQL_NTS);

    char user[50] = { 0 }, pass[256] = { 0 }, priv[50] = { 0 };
    SQLINTEGER aid = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &aid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, user, sizeof(user), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, pass, sizeof(pass), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, priv, sizeof(priv), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Admin a;
        a.admin_id = aid;
        a.username = user;
        a.password = pass;
        a.privilege = priv;
        list.push_back(a);

        // Reset buffer
        memset(user, 0, sizeof(user));
        memset(pass, 0, sizeof(pass));
        memset(priv, 0, sizeof(priv));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= BILLING SYSTEM CRUD =======================

// Thêm Giao dịch mới
bool DataAccessLayer::addTransaction(const BillingSystem& b) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // Lưu ý: transaction_id là PRIMARY KEY. Trong MySQL/ODBC, thường không bind nó 
    // nếu nó là AUTO_INCREMENT. Tuy nhiên, nếu bạn muốn set thủ công:
    // Vì DDL chỉ định nó là PRIMARY KEY (không rõ có phải AUTO_INCREMENT không), ta sẽ chèn nó.
    const char* sql = "INSERT INTO BillingSystem (transaction_id, account_id, method, payment_date) VALUES (?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER tid = b.transaction_id;
    SQLINTEGER aid = b.account_id;

    // Bind tham số (transaction_id, account_id, method, payment_date)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aid, 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)b.method.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_DATETIME, 0, 0, (SQLPOINTER)b.payment_date.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Cập nhật thông tin Giao dịch
bool DataAccessLayer::updateTransaction(const BillingSystem& b) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Cập nhật các trường: account_id, method, payment_date dựa trên transaction_id
    const char* sql = "UPDATE BillingSystem SET account_id=?, method=?, payment_date=? WHERE transaction_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER tid = b.transaction_id;
    SQLINTEGER aid = b.account_id;

    // Bind tham số (account_id, method, payment_date, transaction_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)b.method.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_DATETIME, 0, 0, (SQLPOINTER)b.payment_date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Xóa Giao dịch
bool DataAccessLayer::deleteTransaction(int transaction_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM BillingSystem WHERE transaction_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (transaction_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &transaction_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Lấy Giao dịch theo ID
BillingSystem DataAccessLayer::getTransactionByID(int transaction_id) {
    BillingSystem b;
    if (!connect()) return b;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "SELECT transaction_id, account_id, method, payment_date FROM BillingSystem WHERE transaction_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &transaction_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        char method[50] = { 0 }, pdate[50] = { 0 };
        SQLINTEGER tid = 0, aid = 0;

        SQLGetData(hStmt, 1, SQL_C_SLONG, &tid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_SLONG, &aid, 0, NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, method, sizeof(method), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, pdate, sizeof(pdate), NULL); // Lấy DATETIME dưới dạng chuỗi

        b.transaction_id = tid;
        b.account_id = aid;
        b.method = method;
        b.payment_date = pdate;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return b;
}

// Lấy tất cả Giao dịch
std::vector<BillingSystem> DataAccessLayer::getAllTransactions() {
    std::vector<BillingSystem> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT transaction_id, account_id, method, payment_date FROM BillingSystem", SQL_NTS);

    char method[50] = { 0 }, pdate[50] = { 0 };
    SQLINTEGER tid = 0, aid = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &tid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, &aid, 0, NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, method, sizeof(method), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, pdate, sizeof(pdate), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        BillingSystem b;
        b.transaction_id = tid;
        b.account_id = aid;
        b.method = method;
        b.payment_date = pdate;
        list.push_back(b);

        // Reset buffer
        memset(method, 0, sizeof(method));
        memset(pdate, 0, sizeof(pdate));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}
//========================= BACKUPRESTO========================

// Thêm Bản sao lưu mới
bool DataAccessLayer::addBackup(const BackupResto& b) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // Chèn backup_id, date, file_path, type, admin_id
    const char* sql = "INSERT INTO BackupResto (backup_id, date, file_path, type, admin_id) VALUES (?, ?, ?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER bid = b.backup_id;
    SQLINTEGER aid = b.admin_id;
    // admin_id có thể là NULL, cần xử lý giá trị chỉ báo
    SQLLEN adminIdInd = (b.admin_id == 0) ? SQL_NULL_DATA : SQL_NTS;

    // Bind tham số (backup_id, date, file_path, type, admin_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &bid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_DATE, 0, 0, (SQLPOINTER)b.date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)b.file_path.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)b.type.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aid, 0, &adminIdInd);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Cập nhật Bản sao lưu
bool DataAccessLayer::updateBackup(const BackupResto& b) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    // Cập nhật date, file_path, type, admin_id dựa trên backup_id
    const char* sql = "UPDATE BackupResto SET date=?, file_path=?, type=?, admin_id=? WHERE backup_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER bid = b.backup_id;
    SQLINTEGER aid = b.admin_id;
    SQLLEN adminIdInd = (b.admin_id == 0) ? SQL_NULL_DATA : SQL_NTS;

    // Bind tham số (date, file_path, type, admin_id, backup_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_DATE, 0, 0, (SQLPOINTER)b.date.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)b.file_path.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLPOINTER)b.type.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aid, 0, &adminIdInd);
    SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &bid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Xóa Bản sao lưu
bool DataAccessLayer::deleteBackup(int backup_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM BackupResto WHERE backup_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (backup_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &backup_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Lấy Bản sao lưu theo ID
BackupResto DataAccessLayer::getBackupByID(int backup_id) {
    BackupResto b;
    if (!connect()) return b;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "SELECT backup_id, date, file_path, type, admin_id FROM BackupResto WHERE backup_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &backup_id, 0, NULL);

    if (SQL_SUCCEEDED(SQLExecute(hStmt)) && SQLFetch(hStmt) == SQL_SUCCESS) {
        char date[50] = { 0 }, path[256] = { 0 }, type[50] = { 0 };
        SQLINTEGER bid = 0, aid = 0;
        SQLLEN adminIdLen = 0; // Độ dài thực tế của admin_id

        SQLGetData(hStmt, 1, SQL_C_SLONG, &bid, 0, NULL);
        SQLGetData(hStmt, 2, SQL_C_CHAR, date, sizeof(date), NULL);
        SQLGetData(hStmt, 3, SQL_C_CHAR, path, sizeof(path), NULL);
        SQLGetData(hStmt, 4, SQL_C_CHAR, type, sizeof(type), NULL);
        // Kiểm tra admin_id có phải là NULL không
        SQLGetData(hStmt, 5, SQL_C_SLONG, &aid, 0, &adminIdLen);

        b.backup_id = bid;
        b.date = date;
        b.file_path = path;
        b.type = type;
        // Nếu adminIdLen là SQL_NULL_DATA, đặt admin_id là 0 (hoặc giá trị mặc định của struct)
        b.admin_id = (adminIdLen == SQL_NULL_DATA) ? 0 : aid;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return b;
}

// Lấy tất cả Bản sao lưu
std::vector<BackupResto> DataAccessLayer::getAllBackups() {
    std::vector<BackupResto> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT backup_id, date, file_path, type, admin_id FROM BackupResto", SQL_NTS);

    char date[50] = { 0 }, path[256] = { 0 }, type[50] = { 0 };
    SQLINTEGER bid = 0, aid = 0;
    SQLLEN adminIdLen = 0; // Độ dài thực tế của admin_id

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &bid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, date, sizeof(date), NULL);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, path, sizeof(path), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, type, sizeof(type), NULL);
    SQLBindCol(hStmt, 5, SQL_C_SLONG, &aid, 0, &adminIdLen);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        BackupResto b;
        b.backup_id = bid;
        b.date = date;
        b.file_path = path;
        b.type = type;
        b.admin_id = (adminIdLen == SQL_NULL_DATA) ? 0 : aid; // Kiểm tra NULL khi lấy dữ liệu
        list.push_back(b);

        // Reset buffer
        memset(date, 0, sizeof(date));
        memset(path, 0, sizeof(path));
        memset(type, 0, sizeof(type));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= SYSTEM LOG FUNCTIONS =======================

// Thêm một bản ghi log mới
bool DataAccessLayer::addLog(const SystemLog& log) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // Chèn log_id, user_id, activity, datetime
    // Vì log_id là PRIMARY KEY, nếu nó là AUTO_INCREMENT trong DB thì không cần bind.
    // Giả sử log_id là AUTO_INCREMENT và ta chỉ chèn 3 cột còn lại.
    // LƯU Ý: Nếu log_id không phải AUTO_INCREMENT, bạn cần cung cấp giá trị cho nó. 
    // Tôi tạm thời bỏ qua log_id trong INSERT, giả định nó là AUTO_INCREMENT.
    const char* sql = "INSERT INTO SystemLog (user_id, activity, datetime) VALUES (?, ?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER uid = log.user_id;
    SQLLEN userIdInd = (log.user_id == 0) ? SQL_NULL_DATA : SQL_NTS;

    // Bind tham số (user_id, activity, datetime)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &uid, 0, &userIdInd);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLPOINTER)log.activity.c_str(), 0, NULL);
    SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_DATETIME, 0, 0, (SQLPOINTER)log.datetime.c_str(), 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Lấy tất cả bản ghi log
std::vector<SystemLog> DataAccessLayer::getAllLogs() {
    std::vector<SystemLog> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT log_id, user_id, activity, datetime FROM SystemLog ORDER BY datetime DESC", SQL_NTS);

    char activity[256] = { 0 }, dt[50] = { 0 };
    SQLINTEGER lid = 0, uid = 0;
    SQLLEN userIdLen = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &lid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, &uid, 0, &userIdLen);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, activity, sizeof(activity), NULL);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, dt, sizeof(dt), NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        SystemLog log;
        log.log_id = lid;
        // Kiểm tra user_id có phải là NULL không
        log.user_id = (userIdLen == SQL_NULL_DATA) ? 0 : uid;
        log.activity = activity;
        log.datetime = dt;
        list.push_back(log);

        // Reset buffer
        memset(activity, 0, sizeof(activity));
        memset(dt, 0, sizeof(dt));
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}

// ======================= INVOICE_BILLING FUNCTIONS =======================

// Thêm một liên kết giữa Invoice và Transaction
bool DataAccessLayer::addInvoiceBilling(const Invoice_Billing& ib) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);

    // Chèn invoice_id và transaction_id
    const char* sql = "INSERT INTO Invoice_Billing (invoice_id, transaction_id) VALUES (?, ?)";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    SQLINTEGER iid = ib.invoice_id;
    SQLINTEGER tid = ib.transaction_id;

    // Bind tham số (invoice_id, transaction_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &iid, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tid, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Xóa một liên kết cụ thể giữa Invoice và Transaction (dựa trên Khóa chính kép)
bool DataAccessLayer::deleteInvoiceBilling(int invoice_id, int transaction_id) {
    if (!connect()) return false;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    const char* sql = "DELETE FROM Invoice_Billing WHERE invoice_id=? AND transaction_id=?";
    SQLPrepareA(hStmt, (SQLCHAR*)sql, SQL_NTS);

    // Bind tham số (invoice_id, transaction_id)
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &invoice_id, 0, NULL);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &transaction_id, 0, NULL);

    SQLRETURN ret = SQLExecute(hStmt);
    if (!SQL_SUCCEEDED(ret)) showError(SQL_HANDLE_STMT, hStmt);

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return SQL_SUCCEEDED(ret);
}

// Lấy tất cả các liên kết (thường dùng để hỗ trợ các truy vấn phức tạp hơn ở Business Layer)
std::vector<Invoice_Billing> DataAccessLayer::getAllInvoiceBilling() {
    std::vector<Invoice_Billing> list;
    if (!connect()) return list;
    SQLHSTMT hStmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hConn, &hStmt);
    SQLExecDirectA(hStmt, (SQLCHAR*)"SELECT invoice_id, transaction_id FROM Invoice_Billing", SQL_NTS);

    SQLINTEGER iid = 0, tid = 0;

    // Bind cột
    SQLBindCol(hStmt, 1, SQL_C_SLONG, &iid, 0, NULL);
    SQLBindCol(hStmt, 2, SQL_C_SLONG, &tid, 0, NULL);

    while (SQLFetch(hStmt) == SQL_SUCCESS) {
        Invoice_Billing ib;
        ib.invoice_id = iid;
        ib.transaction_id = tid;
        list.push_back(ib);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    disconnect();
    return list;
}