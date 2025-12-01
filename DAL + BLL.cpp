#include "framework.h"
// Nếu chưa có resource.h, hãy tạo hoặc comment dòng này và tự define ID nếu cần.
// #include "resource.h" 
#include <windows.h>
#include <string>
#include <tchar.h>
#include <commctrl.h> 
#include <sstream> 
#include <iomanip> // Để dùng setprecision
#include "Struct.h" 
#include "BLL.h" 

using namespace std;

#define MAX_LOADSTRING 100

// ========================================================================
// BIẾN TOÀN CỤC VÀ KHỞI TẠO LOGIC
// ========================================================================

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
BOOL AdminMenuWindow(int nCmdShow);
BOOL ManagerMenuWindow(int nCmdShow);
BOOL StudentMenuWindow(int nCmdShow);
// Định nghĩa ID cho Menu Student
#define ID_BTN_STD_VIEW_INVOICE 4001 // UC11
#define ID_BTN_STD_PAY_INVOICE 4002 // UC12
#define ID_BTN_STD_MAINTENANCE 4003 // UC15
#define ID_BTN_STD_LOGOUT 4004 // UC2

HWND hStudentWindow = nullptr;

// Forward declarations
void CreateStudentControls(HWND hWnd);
void OnStudentViewInvoice(HWND hWnd);
void OnStudentPayInvoice(HWND hWnd);
void OnStudentMaintenance(HWND hWnd);

// Định nghĩa ID cho Menu Manager
#define ID_BTN_MGR_SEARCH_ROOM 3001 // UC5
#define ID_BTN_MGR_MANAGE_ROOM 3002 // UC6
#define ID_BTN_MGR_MANAGE_STUDENT 3003 // UC7, UC8
#define ID_BTN_MGR_CONTRACT 3004 // UC9, UC10
#define ID_BTN_MGR_MAINTENANCE 3005 // UC16
#define ID_BTN_MGR_CHECK_EXPIRY 3006 // UC13
#define ID_BTN_MGR_INVOICE 3007 // UC14
#define ID_BTN_MGR_LOGOUT 3008 // UC2

HWND hManagerWindow = nullptr;

// Forward declarations
void CreateManagerControls(HWND hWnd);
void OnManagerSearchRoom(HWND hWnd);
void OnManagerManageRoom(HWND hWnd);
void OnManagerManageStudent(HWND hWnd);
void OnManagerContract(HWND hWnd);
void OnManagerMaintenance(HWND hWnd);
void OnManagerCheckExpiry(HWND hWnd);
void OnManagerInvoice(HWND hWnd);

// Các ID của Controls
#define ID_BTN_LOGIN 1001
#define ID_BTN_LOGOUT 1002
#define ID_TXT_USERNAME 1003
#define ID_TXT_PASSWORD 1004

// ID cho Menu Admin
#define ID_BTN_ADMIN_MANAGE_USER 2001 // UC3
#define ID_BTN_ADMIN_MANAGE_MANAGER 2002 // UC4
#define ID_BTN_ADMIN_REPORT_REVENUE 2003 // UC17
#define ID_BTN_ADMIN_MANAGE_BACKUP 2004 // UC19, UC20
#define ID_BTN_ADMIN_VIEW_LOGS 2005 // UC18
#define ID_BTN_ADMIN_LOGOUT 2006 // UC2

// ID Menu (Mặc định nếu chưa có resource)
#define IDM_ABOUT 104
#define IDM_EXIT 105
#define IDI_DALBLL 107
#define IDI_SMALL 108
#define IDC_DALBLL 109

// Handle của các Controls trên cửa sổ Login
HWND hUsername, hPassword, hLoginButton;

// Handle của cửa sổ Menu Admin
HWND hAdminWindow = nullptr;

// Đối tượng Logic Nghiệp vụ
BusinessLogicLayer bll;

// Thông tin người dùng đang đăng nhập
UserAccount currentUser;

// ========================================================================
// KHAI BÁO HÀM (Forward Declarations)
// ========================================================================
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProcLogin(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcAdmin(HWND, UINT, WPARAM, LPARAM); // Thêm hàm xử lý Admin
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void CreateLoginControls(HWND hWnd);
void CreateAdminControls(HWND hWnd); // Hàm tạo controls Admin
void HandleLogin(HWND hWnd);
void HandleLogout(HWND hWnd);
void ShowMessage(HWND hWnd, const WCHAR* message, const WCHAR* title, UINT type);
string WCharToString(const WCHAR* wstr);
wstring StringToWChar(const string& str);

// Hàm hiển thị cửa sổ Admin
BOOL AdminMenuWindow(int nCmdShow);

// Các hàm xử lý Use Case chi tiết cho Admin (Khung)
void OnAdminManageUsers(HWND hWnd);
void OnAdminManageManagers(HWND hWnd);
void OnAdminReportRevenue(HWND hWnd);
void OnAdminBackupRestore(HWND hWnd);
void OnAdminViewLogs(HWND hWnd);

// ========================================================================
// HÀM TIỆN ÍCH CHUYỂN ĐỔI CHUỖI
// ========================================================================

// Hàm chuyển đổi WCHAR* (Win32) sang std::string (BLL)
string WCharToString(const WCHAR* wstr) {
    if (!wstr) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);
    // Loại bỏ ký tự null cuối cùng nếu có
    if (!strTo.empty() && strTo.back() == '\0') strTo.pop_back();
    return strTo;
}

// Hàm chuyển đổi std::string (BLL) sang WCHAR* (Win32) (Dùng wstring trực tiếp)
wstring StringToWChar(const string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
    // Loại bỏ ký tự null cuối cùng nếu có
    if (!wstrTo.empty() && wstrTo.back() == L'\0') wstrTo.pop_back();
    return wstrTo;
}

// ========================================================================
// HÀM CHÍNH (wWinMain) - ĐIỂM VÀO CỦA CHƯƠNG TRÌNH
// ========================================================================

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Khởi tạo chuỗi tiêu đề (Nếu không dùng Resource String Table thì gán trực tiếp)
    // LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    wcscpy_s(szTitle, L"Quan Ly Ky Tuc Xa");
    wcscpy_s(szWindowClass, L"KtxLoginClass");

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DALBLL));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

// ========================================================================
// ĐĂNG KÝ LỚP CỬA SỔ (MyRegisterClass)
// ========================================================================

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    // Đăng ký lớp cho cửa sổ Login
    WNDCLASSEXW wcexLogin;
    wcexLogin.cbSize = sizeof(WNDCLASSEX);
    wcexLogin.style = CS_HREDRAW | CS_VREDRAW;
    wcexLogin.lpfnWndProc = WndProcLogin;
    wcexLogin.cbClsExtra = 0;
    wcexLogin.cbWndExtra = 0;
    wcexLogin.hInstance = hInstance;
    // Dùng icon mặc định nếu chưa có resource
    wcexLogin.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcexLogin.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcexLogin.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcexLogin.lpszMenuName = nullptr;
    wcexLogin.lpszClassName = L"KtxLoginClass";
    wcexLogin.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExW(&wcexLogin);

    // Đăng ký lớp cho cửa sổ Admin
    WNDCLASSEXW wcexAdmin;
    wcexAdmin.cbSize = sizeof(WNDCLASSEX);
    wcexAdmin.style = CS_HREDRAW | CS_VREDRAW;
    wcexAdmin.lpfnWndProc = WndProcAdmin; // Hàm xử lý thông điệp Admin
    wcexAdmin.cbClsExtra = 0;
    wcexAdmin.cbWndExtra = 0;
    wcexAdmin.hInstance = hInstance;
    wcexAdmin.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcexAdmin.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcexAdmin.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Màu trắng
    wcexAdmin.lpszMenuName = nullptr;
    wcexAdmin.lpszClassName = L"KtxAdminClass";
    wcexAdmin.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassExW(&wcexAdmin);
}

// ========================================================================
// KHỞI TẠO CỬA SỔ (InitInstance) - Vẫn chỉ tạo cửa sổ Login
// ========================================================================

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(L"KtxLoginClass", L"Đăng nhập Hệ thống Ký túc xá",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - 350) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 250) / 2,
        350, 250, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// ========================================================================
// HÀM XỬ LÝ THÔNG ĐIỆP ĐĂNG NHẬP (WndProcLogin)
// ========================================================================

LRESULT CALLBACK WndProcLogin(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateLoginControls(hWnd);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (wmId) {
            case ID_BTN_LOGIN:
                HandleLogin(hWnd);
                break;
            default:
                break;
            }
        }
    }
    break;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ========================================================================
// HÀM XỬ LÝ LOGIC ĐĂNG NHẬP/ĐĂNG XUẤT (UC1, UC2)
// ========================================================================

void HandleLogin(HWND hWnd) {
    WCHAR wUsername[100];
    WCHAR wPassword[100];

    GetWindowTextW(hUsername, wUsername, 100);
    GetWindowTextW(hPassword, wPassword, 100);

    string username = WCharToString(wUsername);
    string password = WCharToString(wPassword);

    currentUser = UserAccount(); // Reset user
    // Gọi hàm login từ BLL
    bool success = bll.login(username, password, currentUser);

    if (success) {
        // ShowMessage(hWnd, L"Đăng nhập thành công!", L"Thành công", MB_OK | MB_ICONINFORMATION);
        ShowWindow(hWnd, SW_HIDE); // Ẩn cửa sổ login

        if (currentUser.role == "Admin") {
            AdminMenuWindow(SW_SHOW);
        }
        else if (currentUser.role == "DormitoryManager") {
            //ShowMessage(hWnd, L"Bạn là Quản lý. Chức năng đang được phát triển.", L"Thông báo", MB_OK);
            ManagerMenuWindow(SW_SHOW); 
        }
        else if (currentUser.role == "Student") {
            //ShowMessage(hWnd, L"Bạn là Sinh viên. Chức năng đang được phát triển.", L"Thông báo", MB_OK);
            StudentMenuWindow(SW_SHOW);
        }
        else {
            ShowMessage(hWnd, L"Vai trò không xác định!", L"Lỗi", MB_OK | MB_ICONERROR);
            ShowWindow(hWnd, SW_SHOW); // Hiện lại login nếu lỗi
        }

    }
    else {
        ShowMessage(hWnd, L"Tên đăng nhập hoặc mật khẩu không đúng.", L"Lỗi Đăng nhập", MB_OK | MB_ICONERROR);
    }
}

void HandleLogout(HWND hWnd) {
    if (currentUser.account_id != 0) {
        bll.addLog(currentUser.account_id, "Logged out.");
        currentUser = UserAccount(); // Reset user

        DestroyWindow(hWnd); // Đóng cửa sổ hiện tại (Admin)
        hAdminWindow = nullptr;

        // Hiện lại cửa sổ đăng nhập
        HWND hLogin = FindWindowW(L"KtxLoginClass", L"Đăng nhập Hệ thống Ký túc xá");
        if (hLogin) {
            SetWindowTextW(hUsername, L"");
            SetWindowTextW(hPassword, L"");
            ShowWindow(hLogin, SW_SHOW);
        }
    }
}

// ========================================================================
// CÁC HÀM TẠO CONTROLS LOGIN
// ========================================================================

void CreateLoginControls(HWND hWnd) {
    CreateWindowW(L"STATIC", L"Tên đăng nhập:", WS_VISIBLE | WS_CHILD | SS_RIGHT, 30, 40, 90, 20, hWnd, NULL, hInst, NULL);
    hUsername = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 130, 38, 170, 25, hWnd, (HMENU)ID_TXT_USERNAME, hInst, NULL);
    CreateWindowW(L"STATIC", L"Mật khẩu:", WS_VISIBLE | WS_CHILD | SS_RIGHT, 30, 80, 90, 20, hWnd, NULL, hInst, NULL);
    hPassword = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL, 130, 78, 170, 25, hWnd, (HMENU)ID_TXT_PASSWORD, hInst, NULL);
    hLoginButton = CreateWindowW(L"BUTTON", L"Đăng nhập", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 130, 130, 100, 30, hWnd, (HMENU)ID_BTN_LOGIN, hInst, NULL);
}

// ========================================================================
// PHẦN 2: CỬA SỔ MENU QUẢN TRỊ VIÊN (ADMIN MENU)
// ========================================================================

// Hàm xử lý thông điệp cho cửa sổ Admin
LRESULT CALLBACK WndProcAdmin(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateAdminControls(hWnd);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (wmId) {
            case ID_BTN_ADMIN_MANAGE_USER:
                OnAdminManageUsers(hWnd);
                break;
            case ID_BTN_ADMIN_MANAGE_MANAGER:
                OnAdminManageManagers(hWnd);
                break;
            case ID_BTN_ADMIN_REPORT_REVENUE:
                OnAdminReportRevenue(hWnd);
                break;
            case ID_BTN_ADMIN_MANAGE_BACKUP:
                OnAdminBackupRestore(hWnd);
                break;
            case ID_BTN_ADMIN_VIEW_LOGS:
                OnAdminViewLogs(hWnd);
                break;
            case ID_BTN_ADMIN_LOGOUT:
                HandleLogout(hWnd);
                break;
            }
        }
    }
    break;

    case WM_CLOSE:
        HandleLogout(hWnd); // Đóng cửa sổ Admin đồng nghĩa với đăng xuất
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Khởi tạo cửa sổ Admin
BOOL AdminMenuWindow(int nCmdShow) {
    if (hAdminWindow) {
        ShowWindow(hAdminWindow, SW_SHOW);
        SetForegroundWindow(hAdminWindow);
        return TRUE;
    }

    hAdminWindow = CreateWindowW(L"KtxAdminClass", L"MENU QUẢN TRỊ VIÊN (ADMIN)",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, // Không cho phép phóng to
        (GetSystemMetrics(SM_CXSCREEN) - 400) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 400) / 2,
        400, 400, nullptr, nullptr, hInst, nullptr);

    if (!hAdminWindow) {
        return FALSE;
    }

    ShowWindow(hAdminWindow, nCmdShow);
    UpdateWindow(hAdminWindow);
    return TRUE;
}

// Tạo Controls cho Admin
void CreateAdminControls(HWND hWnd) {
    int buttonWidth = 300;
    int buttonHeight = 35;
    int marginX = (400 - buttonWidth) / 2 - 10; // Căn giữa
    int startY = 30;
    int spacing = 10;

    wstring welcomeMsg = L"Chào mừng Admin: " + StringToWChar(currentUser.username);
    CreateWindowW(L"STATIC", welcomeMsg.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 5, 360, 20, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"1. Quản lý Tài khoản Người dùng (UC3)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_MANAGE_USER, hInst, NULL);
    startY += buttonHeight + spacing;
    CreateWindowW(L"BUTTON", L"2. Quản lý Thông tin Quản lý Ký túc xá (UC4)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_MANAGE_MANAGER, hInst, NULL);
    startY += buttonHeight + spacing;
    CreateWindowW(L"BUTTON", L"3. Báo cáo Doanh thu Hệ thống (UC17)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_REPORT_REVENUE, hInst, NULL);
    startY += buttonHeight + spacing;
    CreateWindowW(L"BUTTON", L"4. Xem Nhật ký Hệ thống (UC18)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_VIEW_LOGS, hInst, NULL);
    startY += buttonHeight + spacing;
    CreateWindowW(L"BUTTON", L"5. Sao lưu/Khôi phục CSDL (UC19, UC20)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_MANAGE_BACKUP, hInst, NULL);

    startY += buttonHeight + spacing * 2;
    CreateWindowW(L"BUTTON", L"0. Đăng xuất (Logout)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_ADMIN_LOGOUT, hInst, NULL);
}

// ========================================================================
// HÀM XỬ LÝ SỰ KIỆN ADMIN (IMPLEMENTATION)
// ========================================================================

void OnAdminManageUsers(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Quản lý Tài khoản (UC3) đang phát triển.", L"Thông báo", MB_OK);
}

void OnAdminManageManagers(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Quản lý Manager (UC4) đang phát triển.", L"Thông báo", MB_OK);
}

void OnAdminReportRevenue(HWND hWnd) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    // Gọi BLL lấy doanh thu tháng hiện tại
    double revenue = bll.getRevenueReport(st.wMonth, st.wYear);

    wstringstream wss;
    wss << L"Tổng doanh thu tháng " << st.wMonth << L"/" << st.wYear << L" là:\n";
    // Định dạng tiền tệ đơn giản
    wss << fixed << setprecision(0) << revenue << L" VNĐ";

    ShowMessage(hWnd, wss.str().c_str(), L"Báo cáo Doanh thu (UC17)", MB_OK | MB_ICONINFORMATION);
}

void OnAdminBackupRestore(HWND hWnd) {
    if (MessageBoxW(hWnd, L"Bạn có muốn sao lưu cơ sở dữ liệu ngay bây giờ không?", L"Xác nhận Sao lưu", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        bool success = bll.performDatabaseBackup("Full", currentUser.account_id);
        if (success) {
            ShowMessage(hWnd, L"Sao lưu CSDL thành công!", L"Thông báo", MB_OK | MB_ICONINFORMATION);
        }
        else {
            ShowMessage(hWnd, L"Sao lưu CSDL thất bại. Vui lòng kiểm tra kết nối.", L"Lỗi", MB_OK | MB_ICONERROR);
        }
    }
}

void OnAdminViewLogs(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Xem Nhật ký (UC18) đang phát triển.", L"Thông báo", MB_OK);
}

// Hàm hiển thị thông báo chung
void ShowMessage(HWND hWnd, const WCHAR* message, const WCHAR* title, UINT type) {
    MessageBoxW(hWnd, message, title, type);
}

// Implement hàm About callback nếu cần (để tránh lỗi unresolved external)
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    if (message == WM_INITDIALOG) return (INT_PTR)TRUE;
    if (message == WM_COMMAND) {
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}
// Hàm xử lý thông điệp cho cửa sổ Manager
LRESULT CALLBACK WndProcManager(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateManagerControls(hWnd);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (wmId) {
            case ID_BTN_MGR_SEARCH_ROOM:
                OnManagerSearchRoom(hWnd);
                break;
            case ID_BTN_MGR_MANAGE_ROOM:
                OnManagerManageRoom(hWnd);
                break;
            case ID_BTN_MGR_MANAGE_STUDENT:
                OnManagerManageStudent(hWnd);
                break;
            case ID_BTN_MGR_CONTRACT:
                OnManagerContract(hWnd);
                break;
            case ID_BTN_MGR_MAINTENANCE:
                OnManagerMaintenance(hWnd);
                break;
            case ID_BTN_MGR_CHECK_EXPIRY:
                OnManagerCheckExpiry(hWnd);
                break;
            case ID_BTN_MGR_INVOICE:
                OnManagerInvoice(hWnd);
                break;
            case ID_BTN_MGR_LOGOUT:
                HandleLogout(hWnd);
                break;
            }
        }
    }
    break;

    case WM_CLOSE:
        HandleLogout(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Khởi tạo cửa sổ Manager
BOOL ManagerMenuWindow(int nCmdShow) {
    // Đăng ký lớp cửa sổ Manager (nếu chưa)
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcManager;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DALBLL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"KtxManagerClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex);

    if (hManagerWindow) {
        ShowWindow(hManagerWindow, SW_SHOW);
        SetForegroundWindow(hManagerWindow);
        return TRUE;
    }

    hManagerWindow = CreateWindowW(L"KtxManagerClass", L"MENU QUẢN LÝ KÝ TÚC XÁ",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - 450) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 500) / 2,
        450, 500, nullptr, nullptr, hInst, nullptr);

    if (!hManagerWindow) return FALSE;

    ShowWindow(hManagerWindow, nCmdShow);
    UpdateWindow(hManagerWindow);
    return TRUE;
}

// Tạo Controls cho Manager
void CreateManagerControls(HWND hWnd) {
    int buttonWidth = 350;
    int buttonHeight = 35;
    int marginX = (450 - buttonWidth) / 2 - 10;
    int startY = 20;
    int spacing = 10;

    wstring welcomeMsg = L"Chào mừng Quản lý: " + StringToWChar(currentUser.username);
    CreateWindowW(L"STATIC", welcomeMsg.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 5, 410, 20, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"1. Tìm kiếm Thông tin Phòng (UC5)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_SEARCH_ROOM, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"2. Quản lý Phòng (Thêm/Sửa/Xóa) (UC6)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_MANAGE_ROOM, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"3. Quản lý Sinh viên (UC7, UC8)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_MANAGE_STUDENT, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"4. Quản lý Hợp đồng (UC9, UC10)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_CONTRACT, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"5. Kiểm tra Hợp đồng hết hạn (UC13)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_CHECK_EXPIRY, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"6. Lập Hóa đơn Tháng (UC14)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_INVOICE, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"7. Xử lý Yêu cầu Bảo trì (UC16)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_MAINTENANCE, hInst, NULL);

    startY += buttonHeight + spacing * 2;
    CreateWindowW(L"BUTTON", L"0. Đăng xuất (Logout)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_MGR_LOGOUT, hInst, NULL);
}

// ========================================================================
// HÀM XỬ LÝ SỰ KIỆN MANAGER (IMPLEMENTATION)
// ========================================================================

void OnManagerSearchRoom(HWND hWnd) {
    // UC5: Tìm kiếm phòng
    // Demo: Tìm phòng Standard ở tòa nhà A
    vector<Room> rooms = bll.findAvailableRooms("Standard", "A");
    wstringstream wss;
    wss << L"Tìm thấy " << rooms.size() << L" phòng 'Standard' còn trống tại tòa nhà A:\n";
    for (const auto& r : rooms) {
        wss << L"- " << StringToWChar(r.room_number) << L" (Sức chứa: " << r.capacity << L")\n";
    }
    ShowMessage(hWnd, wss.str().c_str(), L"Kết quả Tìm kiếm (UC5)", MB_OK | MB_ICONINFORMATION);
}

void OnManagerManageRoom(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Quản lý Phòng (UC6) đang phát triển.", L"Thông báo", MB_OK);
}

void OnManagerManageStudent(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Quản lý Sinh viên (UC7, UC8) đang phát triển.", L"Thông báo", MB_OK);
}

void OnManagerContract(HWND hWnd) {
    // UC9, UC10: Hợp đồng
    ShowMessage(hWnd, L"Chức năng Quản lý Hợp đồng (UC9, UC10) đang phát triển.", L"Thông báo", MB_OK);
}

void OnManagerCheckExpiry(HWND hWnd) {
    // UC13: Kiểm tra hết hạn
    vector<Student> students = bll.getStudentsWithExpiringContracts(30); // Kiểm tra trong 30 ngày tới

    if (students.empty()) {
        ShowMessage(hWnd, L"Không có sinh viên nào sắp hết hạn hợp đồng trong 30 ngày tới.", L"Thông báo (UC13)", MB_OK);
    }
    else {
        wstringstream wss;
        wss << L"Danh sách sinh viên sắp hết hạn hợp đồng:\n";
        for (const auto& s : students) {
            wss << L"- " << StringToWChar(s.name) << L" (MSSV: " << s.student_id << L")\n";
        }
        ShowMessage(hWnd, wss.str().c_str(), L"Cảnh báo Hết hạn (UC13)", MB_OK | MB_ICONWARNING);
    }
}

void OnManagerInvoice(HWND hWnd) {
    // UC14: Lập hóa đơn tháng
    if (MessageBoxW(hWnd, L"Bạn có muốn lập hóa đơn cho tháng hiện tại không?", L"Xác nhận (UC14)", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        bool result = bll.generateMonthlyInvoices(st.wMonth, st.wYear);

        if (result) {
            ShowMessage(hWnd, L"Đã lập hóa đơn thành công cho các hợp đồng hoạt động.", L"Thông báo", MB_OK | MB_ICONINFORMATION);
        }
        else {
            ShowMessage(hWnd, L"Không có hóa đơn nào được lập (Có thể đã lập rồi hoặc không có hợp đồng).", L"Thông báo", MB_OK);
        }
    }
}

void OnManagerMaintenance(HWND hWnd) {
    ShowMessage(hWnd, L"Chức năng Xử lý Bảo trì (UC16) đang phát triển.", L"Thông báo", MB_OK);
}
// Hàm xử lý thông điệp cho cửa sổ Student
LRESULT CALLBACK WndProcStudent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateStudentControls(hWnd);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (wmId) {
            case ID_BTN_STD_VIEW_INVOICE:
                OnStudentViewInvoice(hWnd);
                break;
            case ID_BTN_STD_PAY_INVOICE:
                OnStudentPayInvoice(hWnd);
                break;
            case ID_BTN_STD_MAINTENANCE:
                OnStudentMaintenance(hWnd);
                break;
            case ID_BTN_STD_LOGOUT:
                HandleLogout(hWnd);
                break;
            }
        }
    }
    break;

    case WM_CLOSE:
        HandleLogout(hWnd);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Khởi tạo cửa sổ Student
BOOL StudentMenuWindow(int nCmdShow) {
    // Đăng ký lớp cửa sổ Student
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcStudent;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_DALBLL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"KtxStudentClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex);

    if (hStudentWindow) {
        ShowWindow(hStudentWindow, SW_SHOW);
        SetForegroundWindow(hStudentWindow);
        return TRUE;
    }

    hStudentWindow = CreateWindowW(L"KtxStudentClass", L"MENU SINH VIÊN",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        (GetSystemMetrics(SM_CXSCREEN) - 400) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 350) / 2,
        400, 350, nullptr, nullptr, hInst, nullptr);

    if (!hStudentWindow) return FALSE;

    ShowWindow(hStudentWindow, nCmdShow);
    UpdateWindow(hStudentWindow);
    return TRUE;
}

// Tạo Controls cho Student
void CreateStudentControls(HWND hWnd) {
    int buttonWidth = 300;
    int buttonHeight = 35;
    int marginX = (400 - buttonWidth) / 2 - 10;
    int startY = 30;
    int spacing = 10;

    wstring welcomeMsg = L"Chào mừng Sinh viên: " + StringToWChar(currentUser.username);
    CreateWindowW(L"STATIC", welcomeMsg.c_str(), WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 5, 360, 20, hWnd, NULL, hInst, NULL);

    CreateWindowW(L"BUTTON", L"1. Xem Hóa đơn cần thanh toán (UC11)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_STD_VIEW_INVOICE, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"2. Thanh toán Hóa đơn (UC12)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_STD_PAY_INVOICE, hInst, NULL);
    startY += buttonHeight + spacing;

    CreateWindowW(L"BUTTON", L"3. Gửi Yêu cầu Bảo trì (UC15)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_STD_MAINTENANCE, hInst, NULL);

    startY += buttonHeight + spacing * 2;
    CreateWindowW(L"BUTTON", L"0. Đăng xuất (Logout)", WS_VISIBLE | WS_CHILD, marginX, startY, buttonWidth, buttonHeight, hWnd, (HMENU)ID_BTN_STD_LOGOUT, hInst, NULL);
}

// ========================================================================
// HÀM XỬ LÝ SỰ KIỆN STUDENT (IMPLEMENTATION)
// ========================================================================

void OnStudentViewInvoice(HWND hWnd) {
    // UC11: Xem hóa đơn pending
    // Giả định account_id trong UserAccount chính là student_id (hoặc cần query map)
    // Để đơn giản cho demo, ta dùng account_id
    vector<Invoice> invoices = bll.getPendingInvoices(currentUser.account_id);

    if (invoices.empty()) {
        ShowMessage(hWnd, L"Bạn không có hóa đơn nào cần thanh toán.", L"Thông báo (UC11)", MB_OK);
    }
    else {
        wstringstream wss;
        wss << L"Danh sách hóa đơn chưa thanh toán:\n";
        for (const auto& inv : invoices) {
            wss << L"- Hóa đơn ID: " << inv.invoice_id
                << L" | Số tiền: " << fixed << setprecision(0) << inv.amount
                << L" | Hạn chót: " << StringToWChar(inv.due_date) << L"\n";
        }
        ShowMessage(hWnd, wss.str().c_str(), L"Hóa đơn (UC11)", MB_OK | MB_ICONINFORMATION);
    }
}

void OnStudentPayInvoice(HWND hWnd) {
    // UC12: Thanh toán hóa đơn
    // Demo: Tự động thanh toán hóa đơn đầu tiên trong danh sách pending
    vector<Invoice> invoices = bll.getPendingInvoices(currentUser.account_id);

    if (invoices.empty()) {
        ShowMessage(hWnd, L"Không có hóa đơn nào để thanh toán.", L"Thông báo", MB_OK);
        return;
    }

    int invoiceIdToPay = invoices[0].invoice_id;
    wstring confirmMsg = L"Bạn có muốn thanh toán hóa đơn ID " + to_wstring(invoiceIdToPay) + L" ngay bây giờ không?";

    if (MessageBoxW(hWnd, confirmMsg.c_str(), L"Xác nhận Thanh toán (UC12)", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        bool success = bll.payInvoice(invoiceIdToPay, "Online Banking", currentUser.account_id);

        if (success) {
            ShowMessage(hWnd, L"Thanh toán thành công!", L"Thông báo", MB_OK | MB_ICONINFORMATION);
        }
        else {
            ShowMessage(hWnd, L"Thanh toán thất bại. Vui lòng thử lại.", L"Lỗi", MB_OK | MB_ICONERROR);
        }
    }
}

void OnStudentMaintenance(HWND hWnd) {
    // UC15: Gửi yêu cầu bảo trì
    // Demo gửi một yêu cầu cứng
    if (MessageBoxW(hWnd, L"Gửi yêu cầu bảo trì: 'Hỏng đèn phòng'?", L"Yêu cầu Bảo trì (UC15)", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        MaintenanceRequest req;
        req.description = "Hỏng đèn phòng";
        req.student_id = currentUser.account_id;
        

        bool success = bll.submitMaintenanceRequest(req);

        if (success) {
            ShowMessage(hWnd, L"Yêu cầu bảo trì đã được gửi.", L"Thông báo", MB_OK | MB_ICONINFORMATION);
        }
        else {
            ShowMessage(hWnd, L"Gửi yêu cầu thất bại.", L"Lỗi", MB_OK | MB_ICONERROR);
        }
    }
}

// ... (Kết thúc file main.cpp)