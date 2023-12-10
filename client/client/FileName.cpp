#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>
#include<ctime>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <chrono>
#include <conio.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Cấu trúc đại diện cho một email
struct Email {
    string nguoiGui;
    string nguoiNhan;
    string nguoiCC;
    string nguoiBCC;
    string subject;
    string noiDung;
    vector<string> tepDinhKem;
};
struct Cau_Truc_File {
    string file_name;

    string file_content;
};
struct Cau_Truc_Email {
    string date;
    string to;
    string from;
    string subject;
    string content;
    string messageID;
    bool kemFile;
    Cau_Truc_File file;
    bool daDoc;  // Trạng thái đã đọc/chưa đọc
};
struct CauHinh {
    std::string ten_nguoi_dung;
    std::string mat_khau;
    std::string mayChuMail;
    int SMTP;
    int POP3;
    int khoiTaoTuDong;
};
// Khởi tạo Winsock
bool khoi_tao_winsock() {
    WSADATA thongTinWSA;
    return WSAStartup(MAKEWORD(2, 2), &thongTinWSA) == 0;
}

// Tạo socket
SOCKET tao_socket() {
    return socket(AF_INET, SOCK_STREAM, 0);
}

// Thiết lập thông tin server SMTP
sockaddr_in thiet_lap_thong_tin_server_SMTP(const char* dia_chi_ip, int cong) {
    sockaddr_in thongTinServer;
    thongTinServer.sin_family = AF_INET;
    thongTinServer.sin_port = htons(cong);
    thongTinServer.sin_addr.s_addr = inet_addr(dia_chi_ip);
    return thongTinServer;
}
// Thiết lập thông tin server POP3
sockaddr_in thiet_lap_thong_tin_server_POP3(const char* dia_chi_ip, int cong) {
    sockaddr_in thongTinServer;
    thongTinServer.sin_family = AF_INET;
    thongTinServer.sin_port = htons(cong);
    thongTinServer.sin_addr.s_addr = inet_addr(dia_chi_ip);
    return thongTinServer;
}
// Kết nối đến server
bool ket_noi_den_server(SOCKET socketClient, const sockaddr_in& thongTinServer) {
    return connect(socketClient, (struct sockaddr*)&thongTinServer, sizeof(thongTinServer)) != SOCKET_ERROR;
}
// Gửi lệnh tới server
void gui_lenh_toi_server(SOCKET socketClient, string lenh) {
    send(socketClient, lenh.c_str(), lenh.length(), 0);
}
// Đọc và in ra phản hồi từ server
string doc_va_in_phan_hoi(SOCKET socketClient) {
    const int bufferSize = 1024;
    vector<char> buffer(bufferSize, 0);
    int bytesRead = recv(socketClient, buffer.data(), buffer.size(), 0);

    if (bytesRead > 0) {
        return string(buffer.data(), bytesRead);
    }
    else {
        cout << "Error in receiving data." << std::endl;
        return ""; // Trả về chuỗi rỗng để biểu thị lỗi
    }
}
// Đóng kết nối và dọn sạch Winsock
void dong_ket_noi_va_don_sach_winsock(SOCKET socketClient) {
    closesocket(socketClient);
    WSACleanup();
}
// Nhập thông tin
Email nhap_thong_tin_email() {
    Email email;
    cout << "\nNguoi gui: ";  getline(cin, email.nguoiGui);

    cout << "\nNguoi nhan (cach nhau boi dau phay ,): ";
    getline(cin, email.nguoiNhan);

    cout << "\nNguoi CC: "; getline(cin, email.nguoiCC);
    if (email.nguoiCC == "")
        cout << "<Enter>";
    cout << "\nNguoi BCC: "; getline(cin, email.nguoiBCC);
    if (email.nguoiBCC == "")
        cout << "<Enter>";
    cout << "\nSubject: "; getline(cin, email.subject);
    cout << "\nNoi dung: "; getline(cin, email.noiDung);
    return email;
}
bool kiem_tra_kich_thuoc_tep(const string& tep, size_t maxSize) {
    ifstream file(tep, ios::binary | ios::ate);
    if (!file.is_open()) {

        return false;
    }
    size_t fileSize = file.tellg();
    file.close();
    return fileSize <= maxSize;
}
string tao_message_id() {
    // Lấy thời gian hiện tại
    time_t now = time(nullptr);
    struct tm timeinfo;
    gmtime_s(&timeinfo, &now);
    // Chuyển đổi thời gian thành chuỗi
    ostringstream time_stream;
    time_stream << std::put_time(&timeinfo, "%Y%m%d%H%M%S");
    // Sử dụng std::random_device để sinh giá trị ngẫu nhiên an toàn hơn
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 99999);  // Giả sử bạn muốn giới hạn giá trị ngẫu nhiên từ 0 đến 99999

    // Kết hợp với giá trị ngẫu nhiên
    std::string message_id = time_stream.str() + "-" + std::to_string(dis(gen)) + "@gmail.com";

    return "Message-ID: <" + message_id + ">";
}
string tao_ky_tu_dac_biet() {
    const string ky_tu_dac_biet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, ky_tu_dac_biet.size() - 1);

    std::string ket_qua;
    for (int i = 0; i < 20; ++i) {  // 20 là độ dài của chuỗi đặt biệt, bạn có thể điều chỉnh
        ket_qua += ky_tu_dac_biet[dis(gen)];
    }

    return ket_qua;
}
string tao_thoi_gian() {
    time_t currentTime;
    time(&currentTime);

    // Chuyển đổi thời gian thành struct tm
    tm localTime;
    localtime_s(&localTime, &currentTime);

    // Tạo chuỗi định dạng
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S +0700", &localTime);

    return string(buffer);
}
string encode_to_base64(const string& input) {
    // Chuyển đổi chuỗi thành vector bytes
    vector<unsigned char> input_bytes(input.begin(), input.end());

    // Sử dụng stringstream để lưu trữ kết quả
    stringstream result;

    // Vòng lặp để chuyển đổi từng 3 bytes thành 4 ký tự Base64
    for (size_t i = 0; i < input_bytes.size(); i += 3) {
        uint32_t group = (input_bytes[i] << 16) | ((i + 1 < input_bytes.size() ? input_bytes[i + 1] : 0) << 8) | (i + 2 < input_bytes.size() ? input_bytes[i + 2] : 0);

        result << static_cast<char>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(group >> 18) & 0x3F]);
        result << static_cast<char>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(group >> 12) & 0x3F]);
        if (i + 1 < input_bytes.size()) {
            result << static_cast<char>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(group >> 6) & 0x3F]);
        }
        else {
            result << '=';
        }
        if (i + 2 < input_bytes.size()) {
            result << static_cast<char>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[group & 0x3F]);
        }
        else {
            result << '=';
        }
    }

    return result.str();
}
void tach_dia_chi(const string& nguoiNhan, vector<string>& danhSachNguoiNhan) {
    istringstream iss(nguoiNhan);
    string token;
    while (getline(iss, token, ',')) {
        danhSachNguoiNhan.push_back(token);
    }
}
void gui_email_toi_server(SOCKET socketClient, Email& email, int tep) {
    // Gửi lệnh MAIL FROM
    string mailFromCommand = "MAIL FROM: <" + email.nguoiGui + ">\r\n";
    gui_lenh_toi_server(socketClient, mailFromCommand);
    // Gửi lệnh RCPT TO cho từng địa chỉ người nhận
    vector<string> danhSachNguoiNhanTo;
    tach_dia_chi(email.nguoiNhan, danhSachNguoiNhanTo);
    for (const auto& recipient : danhSachNguoiNhanTo) {
        string rcptToCommand = "RCPT TO: <" + recipient + ">\r\n";
        gui_lenh_toi_server(socketClient, rcptToCommand);
    }
    // Gửi lệnh RCPT TO cho từng người CC
    vector<string> danhSachNguoiNhanCC;
    tach_dia_chi(email.nguoiCC, danhSachNguoiNhanCC);
    for (const auto& cc : danhSachNguoiNhanCC) {
        string rcptToCommand = "RCPT TO: <" + cc + ">\r\n";
        gui_lenh_toi_server(socketClient, rcptToCommand);
    }
    // Gửi lệnh RCPT TO cho từng người BCC
    vector<string> danhSachNguoiNhanBCC;
    tach_dia_chi(email.nguoiBCC, danhSachNguoiNhanBCC);
    for (const auto& bcc : danhSachNguoiNhanBCC) {
        string rcptToCommand = "RCPT TO: <" + bcc + ">\r\n";
        gui_lenh_toi_server(socketClient, rcptToCommand);
    }

    // Bắt đầu quá trình gửi nội dung email
    string dataCommand = "DATA\r\n";
    gui_lenh_toi_server(socketClient, dataCommand);

    //Gửi boundary
    string boundary = "Content-Type: multipart/mixed; boundary = ------------" + tao_ky_tu_dac_biet() + "\r\n";
    send(socketClient, boundary.c_str(), boundary.length(), 0);
    //Gửi message ID
    string messageTD = tao_message_id() + "\r\n";
    send(socketClient, messageTD.c_str(), messageTD.length(), 0);
    //Gửi Date
    string Date = "Date: " + tao_thoi_gian() + "\r\n";
    send(socketClient, Date.c_str(), Date.length(), 0);
    //Gửi to
    string toCommand = "To: " + email.nguoiNhan + "\r\n";
    send(socketClient, toCommand.c_str(), toCommand.length(), 0);
    //Gửi CC
    string CC = "CC: " + email.nguoiCC + "\r\n";
    send(socketClient, CC.c_str(), CC.length(), 0);
    //Gửi from
    string From = "From: " + encode_to_base64(email.nguoiGui) + " <" + email.nguoiGui + ">\r\n";
    send(socketClient, From.c_str(), From.length(), 0);
    // Gửi tiêu đề email
    string subjectCommand = "Subject: " + email.subject + "\r\n";
    send(socketClient, subjectCommand.c_str(), subjectCommand.length(), 0);
    string boundary1 = "------------" + tao_ky_tu_dac_biet() + "\r\n";
    send(socketClient, boundary1.c_str(), boundary1.length(), 0);
    //Phần này đang làm
    string content1 = "Content-Type: text/plain; charset=UTF-8; format=flowed \r\n";
    send(socketClient, content1.c_str(), content1.length(), 0);
    string content2 = "Content-Transfer-Encoding: N bit \r\n";
    send(socketClient, content2.c_str(), content2.length(), 0);
    // Gửi nội dung email
    string noiDungCommand = email.noiDung + "\r\n";
    send(socketClient, noiDungCommand.c_str(), noiDungCommand.length(), 0);
    // Gửi email có đính kèm tệp
    if (tep == 1) {
        int soLuongTep;
        cout << "Nhap so luong file muon gui: "; cin >> soLuongTep;
        cin.ignore();  // Đọc bỏ dòng new line còn lại từ cin
        string linkTep;
        for (int i = 0; i < soLuongTep; ++i) {
            cout << "Nhap duong link tep " << i + 1 << ": ";
            getline(cin, linkTep);
            if (kiem_tra_kich_thuoc_tep(linkTep, 3 * 1024 * 1024)) {
                cerr << "Kich thuoc tep qua lon (gioi han 3MB): " << linkTep << endl;
                continue;
            }
            email.tepDinhKem.push_back(linkTep);
        }
        for (const auto& linkTep : email.tepDinhKem) {
            if (!kiem_tra_kich_thuoc_tep(linkTep, 3 * 1024 * 1024)) {
                cerr << "Khong the mo tep tin: " << tep << endl;
            }
            if (kiem_tra_kich_thuoc_tep(linkTep, 3 * 1024 * 1024)) {
                ifstream file(linkTep, ios::binary | ios::ate);
                if (!file.is_open()) {
                    return;
                }

                // Lấy tên tệp
                size_t pos = linkTep.find_last_of("\\/");
                string filename = (pos != string::npos) ? linkTep.substr(pos + 1) : linkTep;

                // Tạo header đính kèm
                if (filename == " ")
                {
                    send(socketClient, boundary1.c_str(), boundary1.length(), 0);
                    string attachmentHeader = "Content-Type: text/plain; name=\"" + filename + "\"\r\n";
                    attachmentHeader += "Content-Disposition: attachment; filename=\"" + filename + "\"\r\n";
                    attachmentHeader += "Content-Transfer-Encoding: base64\r\n\r\n";
                    send(socketClient, attachmentHeader.c_str(), attachmentHeader.length(), 0);
                    // Mã hóa tên tệp
                    string link_file = encode_to_base64(linkTep);
                    send(socketClient, link_file.c_str(), link_file.length(), 0);
                }
                // Đóng tệp tin
                file.close();
                cout << "Da gui tep tin dinh kem." << endl;
            }
        }
    }
    // Kết thúc quá trình gửi email
    string endDataCommand = ".\r\n";
    gui_lenh_toi_server(socketClient, endDataCommand);

    cout << "Da gui email thanh cong." << endl;

}


// POP 3



// Hàm kết nối đến server POP3
bool ket_noi_den_server_POP3(SOCKET socketClient, const sockaddr_in& thongTinServer) {
    if (ket_noi_den_server(socketClient, thongTinServer)) {
        cerr << "Khong the ket noi den server." << endl;
        return false;
    }

    // Nhận và in ra phản hồi kết nối từ server
    string ketQua = doc_va_in_phan_hoi(socketClient);
    cout << "Ket qua ket noi: " << ketQua << endl;

    // Kiểm tra xem kết nối có thành công không
    if (ketQua.substr(0, 3) != "+OK") {
        cerr << "Ket noi den server POP3 that bai." << endl;
        return false;
    }

    return true;
}
// Hàm đăng nhập vào tài khoản POP3
bool dang_nhap_POP3(SOCKET socketClient, const string& ten_nguoi_dung, const string& mat_khau) {
    // Gửi yêu cầu đăng nhập
    string lenhUSER = "USER " + ten_nguoi_dung + "\r\n";
    gui_lenh_toi_server(socketClient, lenhUSER);

    // Nhận và in ra phản hồi từ server
    string ketQuaUSER = doc_va_in_phan_hoi(socketClient);
    cout << "Ket qua USER: " << ketQuaUSER << endl;

    // Kiểm tra xem yêu cầu USER có thành công không
    if (ketQuaUSER.substr(0, 3) != "+OK") {
        cerr << "Yeu cau USER that bai." << endl;
        return false;
    }

    // Gửi yêu cầu nhập mật khẩu
    string lenhPASS = "PASS " + mat_khau + "\r\n";
    gui_lenh_toi_server(socketClient, lenhPASS);

    // Nhận và in ra phản hồi từ server
    string ketQuaPASS = doc_va_in_phan_hoi(socketClient);
    cout << "Ket qua PASS: " << ketQuaPASS << endl;

    // Kiểm tra xem yêu cầu PASS có thành công không
    if (ketQuaPASS.substr(0, 3) != "+OK") {
        cerr << "Yeu cau PASS that bai." << endl;
        return false;
    }

    return true;
}
// Hàm đóng kết nối và đơn sạch Winsock theo giao thức POP3
void dong_ket_noi_va_don_sach_winsock_POP3(SOCKET socketClient) {
    // Gửi lệnh QUIT
    string lenhQUIT = "QUIT\r\n";
    gui_lenh_toi_server(socketClient, lenhQUIT);

    // Nhận và in ra phản hồi từ server
    string ketQuaQUIT = doc_va_in_phan_hoi(socketClient);
    cout << "Ket qua QUIT: " << ketQuaQUIT << endl;

    // Đóng kết nối và dọn sạch Winsock
    closesocket(socketClient);
    WSACleanup();
}
// Hàm xóa kí tự thừa
void xoa_ki_tu_thua(string& chuoi) {
    size_t vi_tri = 0;
    while ((vi_tri = chuoi.find("\r\n", vi_tri)) != std::string::npos) {
        chuoi.erase(vi_tri, 2); // Xoá 2 ký tự "\r\n"
    }
}
// Hàm bộ trợ lấy gmail To
vector<string> lay_gia_tri_tu_chuoi(const string& s) {
    vector<string> ket_qua;
    stringstream ss(s);
    string phan_tu;

    while (getline(ss, phan_tu, ',')) {
        // Lo?i b? d?u cách tr?ng ? ??u và cu?i m?i ph?n t?
        size_t dau_cach_dau = phan_tu.find_first_not_of(" \t");
        size_t dau_cach_cuoi = phan_tu.find_last_not_of(" \t");

        if (dau_cach_dau != string::npos && dau_cach_cuoi != string::npos) {
            ket_qua.push_back(phan_tu.substr(dau_cach_dau, dau_cach_cuoi - dau_cach_dau + 1));
        }
    }
    return ket_qua;
}
// Hàm gửi lệnh và nhận phản hồi từ server
string gui_va_nhan_phan_hoi(SOCKET socketClient, const string& lenh) {
    gui_lenh_toi_server(socketClient, lenh);
    return doc_va_in_phan_hoi(socketClient);
}
// Hàm đếm số lượng email
int dem_so_luong_email(const string& listResponse) {
    stringstream ss(listResponse);
    string line;
    int count = 0;
    // Bỏ qua dòng đầu tiên vì nó chứa thông tin đầu danh sách
    getline(ss, line);

    // Đếm số lượng dòng còn lại, mỗi dòng đại diện cho một email trong danh sách
    while (getline(ss, line))
    {
        if (line == ".\r")
            return count;
        else
            count++;
    }
    return count;
}
// Hàm lấy danh sách các email từ server POP3
void lay_danh_sach_email_POP3(SOCKET socketClient, int& so_luong_email) {
    // Gửi lệnh LIST
    string lenhLIST = "LIST\r\n";
    gui_lenh_toi_server(socketClient, lenhLIST);
    // Nhận và in ra danh sách các email từ server
    string danhSachEmail = doc_va_in_phan_hoi(socketClient);
    so_luong_email = dem_so_luong_email(danhSachEmail);
    danhSachEmail = danhSachEmail.substr(5, danhSachEmail.length());
    cout << "Danh sach cac email:\n" << danhSachEmail << endl;
}
// Hàm Kiểm Tra file
bool kiem_tra_file(const string& emailContent) {
    return emailContent.find("base") != string::npos;
}
// Hàm giải mã base64
string base64_decode(const std::string& encoded_string) {
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string decoded_string;
    int in_len = static_cast<int>(encoded_string.size());
    int i = 0;

    while (in_len-- && (encoded_string[i] != '=') &&
        (isalnum(encoded_string[i]) || (encoded_string[i] == '+') || (encoded_string[i] == '/'))) {
        char c1 = base64_chars.find(encoded_string[i++]);
        char c2 = base64_chars.find(encoded_string[i++]);
        char c3 = base64_chars.find(encoded_string[i++]);
        char c4 = base64_chars.find(encoded_string[i++]);

        char d1 = (c1 << 2) | (c2 >> 4);
        char d2 = ((c2 & 0x0f) << 4) | (c3 >> 2);
        char d3 = ((c3 & 0x03) << 6) | c4;

        decoded_string += d1;
        if (c3 != 64) {
            decoded_string += d2;
        }
        if (c4 != 64) {
            decoded_string += d3;
        }
    }

    return decoded_string;
}
// Hàm Lấy nội dung
string lay_noi_dung(const string& emailContent, const string& Tag) {
    stringstream ss(emailContent);
    string line;
    string extracted;

    while (getline(ss, line)) {
        size_t found = line.find(Tag);
        if (found != string::npos) {
            size_t start = found + Tag.length(); // Bắt đầu từ vị trí sau Tag
            size_t end = line.find("\"", start);
            extracted = line.substr(start, end - start);
            break; // Once found, stop further iteration
        }
    }

    return extracted;
}
// Hàm lấy messageID
string lay_MessageID(const string& emailContent, const string& Tag) {
    stringstream ss(emailContent);
    string line;
    string extracted;

    while (getline(ss, line)) {
        size_t found = line.find(Tag);
        if (found != string::npos) {
            size_t start = found + Tag.length(); // Bắt đầu từ vị trí sau Tag
            size_t end = line.find(">", start);
            extracted = line.substr(start, end - start);
            break; // Once found, stop further iteration
        }
    }

    return extracted;
}
// tao email chứa nội dung
Cau_Truc_Email tao_email(const string& emailContent) {
    Cau_Truc_Email email;

    // Thiết lập trạng thái chưa đọc ban đầu
    email.daDoc = false;
    if (kiem_tra_file(emailContent))
    {
        email.file.file_name = lay_noi_dung(emailContent, "filename=\"");
        email.kemFile = TRUE;
    }
    else
        email.kemFile = FALSE;
    // Tạo một stringstream từ nội dung email
    stringstream ss(emailContent);
    string line;
    string boundary = lay_noi_dung(emailContent, "boundary=\"");
    email.messageID = lay_MessageID(emailContent, "Message-ID: <");
    bool vi_tri_Content = false; // Đánh dấu khi đã đến phần nội dung của email
    int vi_tri_File = 0;
    while (getline(ss, line)) {
        // Kiểm tra xem dòng này có bắt đầu bằng các từ khóa như "Date:", "From:", "Subject:", "Content:"
        // để xác định từng phần của email và gán vào cấu trúc email tương ứng
        if (line.substr(0, 6) == "Date: ") {
            email.date = line.substr(6); // Bỏ qua "Date: " và lấy nội dung ngày
        }
        else if (line.substr(0, 4) == "To: ") {
            email.to = line.substr(4); // Bỏ qua "From: " và lấy nội dung người gửi
        }
        else if (line.substr(0, 6) == "From: ") {
            email.from = line.substr(6); // Bỏ qua "From: " và lấy nội dung người gửi
        }

        else if (line.substr(0, 9) == "Subject: ") {
            email.subject = line.substr(9); // Bỏ qua "Subject: " và lấy nội dung tiêu đề
        }
        else if (line.find("bit") != string::npos) {
            vi_tri_Content = true;
        }
        else if (line == ".\r" || line.find(boundary) != string::npos && kiem_tra_file(emailContent)) {
            // Kết thúc phần nội dung khi gặp dòng chỉ chứa dấu chấm
            vi_tri_Content = 0;
        }
        else if (vi_tri_Content == 1) {
            email.content += line + "\n"; // Gán dòng vào phần nội dung của email
        }
        else if (line.find("base64") != string::npos) {
            vi_tri_File = true;
        }
        else if (line == ".\r" || line.find(boundary) != string::npos) {
            // Kết thúc phần nội dung khi gặp dòng chỉ chứa dấu chấm
            vi_tri_File = 0;
        }
        else if (vi_tri_File == 1) {
            email.file.file_content += line + "\n";;// Gán dòng vào phần nội dung của email
        }
    }
    xoa_ki_tu_thua(email.content);
    xoa_ki_tu_thua(email.file.file_content);
    return email;
}
// in thông tin
void in_thong_tin_email(Cau_Truc_Email& email) {
    cout << "Date: " << email.date << endl;
    cout << "From: " << email.from << endl;
    cout << "To: " << email.to << endl;
    cout << "Subject: " << email.subject << endl;
    cout << "Content:\n" << email.content << endl;
    if (email.kemFile == 1)
    {
        int tmp = 0;
        cout << "email co dinh kem file!\n";
        cout << "Ban co muon xem noi dung file dinh kem khong? (1: co; 2: khong)\n";
        cin >> tmp;
        if (tmp == 1)
            cout << base64_decode(email.file.file_content);
    }
    email.daDoc = true;
}
// hàm chuyển thành string để lưu
string chuyen_thanh_string(const Cau_Truc_Email& email) {
    stringstream ss;

    // Ghi từng thành phần của struct Cau_Truc_Email vào stringstream
    ss << "Date: " << email.date << "\n";
    ss << "From: " << email.from << "\n";
    ss << "To: " << email.to << "\n";
    ss << "Subject: " << email.subject << "\n";
    ss << "Content:\n" << email.content << "\n";

    // Ghi thông tin về file nếu có
    if (email.kemFile) {
        ss << "File Name: " << email.file.file_name << "\n";
        ss << "File Content:\n" << email.file.file_content << "\n";
    }

    // Chuyển stringstream về dạng string và trả về
    return ss.str();
}
// Hàm lấy tên gmail <--> trong from
string lay_thong_tin_from(const string& chuoi, const string& batDau, const string& ketThuc) {
    size_t viTriBatDau = chuoi.find(batDau);
    if (viTriBatDau == string::npos) {
        cerr << "Khong tim thay chuoi bat dau." << endl;
        return "";
    }

    viTriBatDau += batDau.length();
    size_t viTriKetThuc = chuoi.find(ketThuc, viTriBatDau);
    if (viTriKetThuc == string::npos) {
        cerr << "Khong tim thay chuoi ket thuc." << endl;
        return "";
    }

    return chuoi.substr(viTriBatDau, viTriKetThuc - viTriBatDau);
}
// Hàm kiểm tra chuỗi a có trong chuỗi b hay không
int kiemTraChuoiTonTai(string chuoiChua[], string& chuoiCanTim, int n) {
    for (int i = 0; i < n; i++)
    {
        if (chuoiCanTim.find(chuoiChua[i]) != string::npos) {

            return 1;
        }
    }
    return 0;

}
// Ham Kiểm tra chuỗi để lọc dựa vao folder spam
bool kiemTraThongTinTapTin_spam(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\spam\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Ham Kiểm tra chuỗi để lọc dựa vao folder quan trọng
bool kiemTraThongTinTapTin_qtrong(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\quantrong\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Ham Kiểm tra chuỗi để lọc dựa vao folder sinh viên
bool kiemTraThongTinTapTin_sinhvien(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\sinhvien\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Ham Kiểm tra chuỗi để lọc dựa vao folder việc làm
bool kiemTraThongTinTapTin_vieclam(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\vieclam\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Ham Kiểm tra chuỗi để lọc dựa vao folder bình thường
bool kiemTraThongTinTapTin_binhthuong(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\binhthuong\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Hàm lọc tệp tin 
void loc_tep_tin(const string& noiDungEmail, Cau_Truc_Email email)
{
    string name_of_mail = email.messageID.substr(0, 18);
    string spam = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\spam\\" + name_of_mail + ".txt";
    string sinhvien = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\sinhvien\\" + name_of_mail + ".txt";
    string quantrong = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\quantrong\\" + name_of_mail + ".txt";
    string vieclam = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\vieclam\\" + name_of_mail + ".txt";
    string binhthuong = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\binhthuong\\" + name_of_mail + ".txt";

    string nguoi_gui_spam = lay_thong_tin_from(email.from, "<", ">");
    string nguoi_gui_sinhvien = lay_thong_tin_from(email.from, "<", ">");
    string nguoi_gui_qtrong = lay_thong_tin_from(email.from, "<", ">");
    string nguoi_gui_vieclam = lay_thong_tin_from(email.from, "<", ">");

    string spam_subject = email.subject;
    string sinhvien_subject = email.subject;
    string qtrong_subject = email.subject;
    string vieclam_subject = email.subject;

    string spam_noi_dung = email.content;
    string sinhvien_noi_dung = email.content;
    string qtrong_noi_dung = email.content;
    string vieclam_noi_dung = email.content;


    string ktspam[3] = { "virus", "vay von", "khung bo" };
    string ktsinhvien[3] = { "hoc phi", "sinh vien", "truong hoc" };
    string ktquantrong[3] = { "help", "gap", "dealine" };
    string ktvieclam[3] = { "don xin viec", "viec lam", "trung tuyen" };
    int n = 0;
    if (kiemTraChuoiTonTai(ktspam, nguoi_gui_spam, 3) == 1 || kiemTraChuoiTonTai(ktspam, spam_subject, 3) == 1 || kiemTraChuoiTonTai(ktspam, spam_noi_dung, 3) == 1)
    {
        if (kiemTraThongTinTapTin_spam(email.messageID.substr(0, 18)) == 1) {
            ofstream outFile(spam, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
            if (outFile.is_open()) {
                outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
                outFile.close(); // Đóng tệp tin
                cout << "Da ghi noi dung vao tep tin .txt." << endl;
            }
            else {
                cerr << "Khong the mo hoac tao tep tin .txt." << std::endl;
            }
        }
        n++;
    }
    if (kiemTraChuoiTonTai(ktsinhvien, nguoi_gui_sinhvien, 3) == 1 || kiemTraChuoiTonTai(ktsinhvien, sinhvien_subject, 3) == 1 || kiemTraChuoiTonTai(ktsinhvien, sinhvien_noi_dung, 3) == 1)
    {
        if (kiemTraThongTinTapTin_sinhvien(email.messageID.substr(0, 18)) == 1) {
            ofstream outFile(sinhvien, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
            if (outFile.is_open()) {
                outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
                outFile.close(); // Đóng tệp tin
                cout << "Da ghi noi dung vao tep tin .txt." << endl;
            }
        }
        n++;
    }
    if (kiemTraChuoiTonTai(ktquantrong, nguoi_gui_qtrong, 3) == 1 || kiemTraChuoiTonTai(ktquantrong, qtrong_subject, 3) == 1 || kiemTraChuoiTonTai(ktquantrong, qtrong_noi_dung, 3) == 1)
    {
        if (kiemTraThongTinTapTin_qtrong(email.messageID.substr(0, 18)) == 1) {
            ofstream outFile(quantrong, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
            if (outFile.is_open()) {
                outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
                outFile.close(); // Đóng tệp tin
                cout << "Da ghi noi dung vao tep tin .txt." << endl;
            }
        }
        n++;
    }
    if (kiemTraChuoiTonTai(ktvieclam, nguoi_gui_vieclam, 3) == 1 || kiemTraChuoiTonTai(ktvieclam, vieclam_subject, 3) == 1 || kiemTraChuoiTonTai(ktvieclam, vieclam_noi_dung, 3) == 1)
    {
        if (kiemTraThongTinTapTin_vieclam(email.messageID.substr(0, 18)) == 1) {
            ofstream outFile(vieclam, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
            if (outFile.is_open()) {
                outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
                outFile.close(); // Đóng tệp tin
                cout << "Da ghi noi dung vao tep tin .txt." << endl;
            }
        }
        n++;
    }
    if (n == 0)
    {
        if (kiemTraThongTinTapTin_binhthuong(email.messageID.substr(0, 18)) == 1) {
            ofstream outFile(binhthuong, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
            if (outFile.is_open()) {
                outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
                outFile.close(); // Đóng tệp tin
                cout << "Da ghi noi dung vao tep tin .txt." << endl;
            }
        }
    }
}


// Hàm ghi nội dung email vào tệp tin mới
void ghi_noi_dung_vao_tep_tin(const string& noiDungEmail, Cau_Truc_Email email)
{
    string name_of_mail = email.messageID.substr(0, 18);
    string duong_dan_toi_vi_tri_luu_email = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\all\\" + name_of_mail + ".txt";
    ofstream outFile(duong_dan_toi_vi_tri_luu_email, ios::app); // Tạo hoặc mở tệp tin để ghi vào cuối
    if (outFile.is_open()) {
        outFile << noiDungEmail; // Ghi nội dung email vào tệp tin
        outFile.close(); // Đóng tệp tin
        cout << "Da ghi noi dung vao tep tin .txt." << endl;
    }
    else {
        cerr << "Khong the mo hoac tao tep tin .txt." << std::endl;
    }
    loc_tep_tin(noiDungEmail, email);
}
// Ham Kiểm tra chuỗi để lọc dựa vao folder all
bool kiemTraThongTinTapTin_all(string ssid) {
    std::string duongDanTapTin = "C:\\Users\\ADMIN\\source\\repos\\socketClient\\socketClient\\inbox\\all\\";
    for (const auto& entry : filesystem::directory_iterator(duongDanTapTin)) {
        if (filesystem::is_regular_file(entry.path())) {
            entry.path().filename();
            if (entry.path().filename() == ssid + ".txt")
                return 0;
        }
    }
    return 1;
}
// Hàm lấy nội dung của một email theo chỉ số từ server POP3
Cau_Truc_Email lay_noi_dung_email_POP3(SOCKET socketClient, int chi_so_email) {
    // Gửi lệnh RETR
    string lenhRETR = "RETR " + to_string(chi_so_email) + "\r\n";
    gui_lenh_toi_server(socketClient, lenhRETR);

    // Nhận và in ra nội dung của email từ server
    string noiDungEmail = doc_va_in_phan_hoi(socketClient);
    Cau_Truc_Email email = tao_email(noiDungEmail);
    if (kiemTraThongTinTapTin_all(email.messageID.substr(0, 18)) == 1)
        ghi_noi_dung_vao_tep_tin(chuyen_thanh_string(email), email);
    return email;
}
// tải toàn bộ email về
void tai_toan_bo_email_POP3(SOCKET socketClient, int& so_luong_email)
{
    for (int i = 1; i <= so_luong_email; i++)
        lay_noi_dung_email_POP3(socketClient, i);
}

// Hàm Lấy danh sách trong folder
vector<string> listTxtFiles(const string& folderPath) {
    vector<string> txtFiles;
    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".txt") {
            txtFiles.push_back(entry.path().filename().string());
        }
    }
    return txtFiles;
}
// Ham kiểm tra mở file folder
void displayFileInfo(const string& filePath) {
    ifstream file(filePath);
    if (file.is_open()) {
        string content((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
        cout << "File: " << filePath << "\nContent:\n" << content << endl;
        file.close();
    }
    else {
        cout << "khong the mo file: " << filePath << endl;
    }
}
void docFileConfig(const char* filePath, CauHinh& cauHinh, Filter& filter) {
    pugi::xml_document doc;
    if (!doc.load_file(filePath)) {
        std::cerr << "Khong the mo file config XML." << std::endl;
        return;
    }

    // Đọc thông tin từ phần General
    auto generalNode = doc.child("General");
    cauHinh.tenNguoiDung = generalNode.child("Username").text().as_string();
    cauHinh.matKhau = generalNode.child("Password").text().as_string();
    cauHinh.mayChuMail = generalNode.child("MailServer").text().as_string();
    cauHinh.SMTP = generalNode.child("SMTP").text().as_int();
    cauHinh.POP3 = generalNode.child("POP3").text().as_int();
    cauHinh.autoload = generalNode.child("Autoload").text().as_int();

    // Đọc thông tin từ phần Filter
    auto filterNode = doc.child("Filter");
    for (auto fromNode : filterNode.child("From").children("item")) {
        filter.from.push_back(fromNode.text().as_string());
    }

    for (auto subjectNode : filterNode.child("Subject").children("item")) {
        filter.subject.push_back(subjectNode.text().as_string());
    }

    for (auto contentNode : filterNode.child("Content").children("item")) {
        filter.content.push_back(contentNode.text().as_string());
    }

    for (auto spamNode : filterNode.child("Spam").children("item")) {
        filter.spam.push_back(spamNode.text().as_string());
    }
}
int main()
{
    int cin_ignore = 0;
    while (true)
    {
        int mn = 0;
        cout << "\nChon Bang Menu: ";
        cout << "\n1. Gui Gmail.";
        cout << "\n2. De xem danh sach va tai truc tiep cac email tu sever.";
        cout << "\n3. Doc email da tai ve.";
        cout << "\n4. Tai email tu dong.";
        cout << "\n5. Thoat.";
        cout << "\nchon: "; cin >> mn;
        if (mn == 1)
        {
            cout << "\nBan da chon Gui Gmail.\n";
            // Kết nối máy chủ
            if (!khoi_tao_winsock()) {
                cerr << "Khong the khoi tao Winsock." << endl;
                return 1;
            }
            SOCKET socketClientSMTP = tao_socket();
            if (socketClientSMTP == INVALID_SOCKET)
            {
                cerr << "Khong the tao socket." << endl;
                dong_ket_noi_va_don_sach_winsock(socketClientSMTP);
                return 1;
            }

            const char* smtp = "127.0.0.1";

            int SMTP = 443;

            sockaddr_in thongTinServerSMTP = thiet_lap_thong_tin_server_SMTP(smtp, SMTP);
            if (!ket_noi_den_server(socketClientSMTP, thongTinServerSMTP)) {
                cerr << "Khong the ket noi den server SMTP." << endl;
                dong_ket_noi_va_don_sach_winsock(socketClientSMTP);
                return 0;
            }
            else
                cerr << "Da ket noi den server SMTP." << endl;

            string hello = "EHLO example.com\r\n";
            gui_lenh_toi_server(socketClientSMTP, hello);
            if (cin_ignore == 0)
                cin.ignore();
            // Nhập Gmail
            Email email = nhap_thong_tin_email();
            // Gửi email đến server
            int tep;
            cout << "\nBan co muon dinh kem tep khong!(1: co, 2: khong): "; cin >> tep;
            gui_email_toi_server(socketClientSMTP, email, tep);
            // Chờ 5 giây giữa hai lần gửi để giả định là có một khoảng thời gian giữa các tác vụ
            this_thread::sleep_for(chrono::seconds(5));
            // Đóng kết nối
            dong_ket_noi_va_don_sach_winsock(socketClientSMTP);
        }

        if (mn == 2)
        {
            cout << "\nBan da chon: xem danh sach va tai truc tiep cac email tu sever.\n";
            // Kết nối máy chủ
            if (!khoi_tao_winsock()) {
                cerr << "Khong the khoi tao Winsock." << endl;
                return 1;
            }
            const char* pop3 = "127.0.0.1";
            int POP3 = 578;
            sockaddr_in thongTinServerPOP3 = thiet_lap_thong_tin_server_POP3(pop3, POP3);
            SOCKET socketClientPOP3 = tao_socket();
            if (socketClientPOP3 == INVALID_SOCKET)
            {
                cerr << "Khong the tao socket." << endl;
                dong_ket_noi_va_don_sach_winsock(socketClientPOP3);
                return 1;
            }

            if (!ket_noi_den_server(socketClientPOP3, thongTinServerPOP3)) {
                cerr << "Khong the ket noi den server POP3." << endl;
                dong_ket_noi_va_don_sach_winsock(socketClientPOP3);
                return 0;
            }
            else
                cerr << "Da ket noi den server POP3." << endl;
            if (!ket_noi_den_server_POP3(socketClientPOP3, thongTinServerPOP3)) {
                cerr << "Khong the ket noi den server POP3." << endl;
                dong_ket_noi_va_don_sach_winsock_POP3(socketClientPOP3);
                return 1;
            }
            //Nhập thông tin đăng nhập
            string ten_nguoi_dung, mat_khau;
            cout << "\nDang nhap Tai Khoan";
            cout << "\nTen Tai Khoan: "; cin >> ten_nguoi_dung;
            cout << "\nMat Khau: "; cin >> mat_khau;

            // Đăng nhập
            if (!dang_nhap_POP3(socketClientPOP3, ten_nguoi_dung, mat_khau)) {
                cerr << "Dang nhap that bai." << endl;
                dong_ket_noi_va_don_sach_winsock_POP3(socketClientPOP3);
                return 1;
            }
            while (true)
            {
                int so_luong_email = 0;
                lay_danh_sach_email_POP3(socketClientPOP3, so_luong_email);


                int lua_chon = 0;
                cout << "1: xem email.\n2: tai toan bo email chua doc tu sever ve.";
                cout << "\nchon: "; cin >> lua_chon;
                if (lua_chon == 1)
                {
                    cout << "Ban muon xem email thu: ";
                    // Lấy nội dung của email thứ nhất
                    int chi_so_email = 0;  // Bạn có thể thay đổi chỉ số email tùy theo danh sách
                    cin >> chi_so_email;
                    Cau_Truc_Email email = lay_noi_dung_email_POP3(socketClientPOP3, chi_so_email);
                    in_thong_tin_email(email);

                }
                else if (lua_chon == 2)
                {
                    cout << "Ban da chon tai toan bo email chua doc tu sever ve: ";
                    tai_toan_bo_email_POP3(socketClientPOP3, so_luong_email);
                }
                // Chờ 5 giây giữa hai lần gửi để giả định là có một khoảng thời gian giữa các tác vụ
                this_thread::sleep_for(chrono::seconds(5));
                int tieptuc = 0;
                cout << "\nBan co muon tiep tuc voi tai khoan nay!(1: co, !=1: khong): "; cin >> tieptuc;
                if (tieptuc != 1) break;
            }
        }

        if (mn == 3)
        {
            cout << "\nBan da chon: doc email da tai ve.\n";
            int LUA_CHON;
            cout << "\nXin hay chon muc: ";
            cout << "\n1. tat ca email da tai ve.\n2. theo danh sach loc.";
            cout << "\nchon: "; cin >> LUA_CHON;
            if (LUA_CHON == 1) {
                string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all";
                vector<string> txtFiles = listTxtFiles(folderPath);
                cout << "Total number of txt files: " << txtFiles.size() << endl;
                cout << "List of txt files:" << endl;

                vector<string> tentep;
                for (const auto& fileName : txtFiles) {

                    cout << fileName << endl;
                    tentep.push_back(fileName);
                }

                cout << "Enter the number of file txt you want to view: ";
                int selectedFile;
                cin >> selectedFile;

                string filePath = folderPath + "/" + tentep[selectedFile - 1];

                if (filesystem::exists(filePath)) {
                    displayFileInfo(filePath);
                }
                else {
                    cout << "File not found." << endl;
                }
            }
            else if (LUA_CHON == 2) {
                cout << "\nBan da chon: doc email theo danh sach loc.";
                int LUA_CHON1;
                cout << "\nXin hay chon muc: ";
                cout << "\n1. doc email spam.\n2. doc email sinhvien.\n3. doc email quantrong.\n4 doc email viec lam.\n5. doc email binh thuong.";
                cout << "\nchon: "; cin >> LUA_CHON1;
                if (LUA_CHON1 == 1) {
                    string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all/spam";
                    vector<string> txtFiles = listTxtFiles(folderPath);
                    cout << "Total number of txt files: " << txtFiles.size() << endl;
                    cout << "List of txt files:" << endl;

                    vector<string> tentep;
                    for (const auto& fileName : txtFiles) {

                        cout << fileName << endl;
                        tentep.push_back(fileName);
                    }

                    cout << "Enter the number of file txt you want to view: ";
                    int selectedFile;
                    cin >> selectedFile;

                    string filePath = folderPath + "/" + tentep[selectedFile - 1];

                    if (filesystem::exists(filePath)) {
                        displayFileInfo(filePath);
                    }
                    else {
                        cout << "File not found." << endl;
                    }
                }
                if (LUA_CHON1 == 2) {
                    string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all/sinhvien";
                    vector<string> txtFiles = listTxtFiles(folderPath);
                    cout << "Total number of txt files: " << txtFiles.size() << endl;
                    cout << "List of txt files:" << endl;

                    vector<string> tentep;
                    for (const auto& fileName : txtFiles) {

                        cout << fileName << endl;
                        tentep.push_back(fileName);
                    }

                    cout << "Enter the number of file txt you want to view: ";
                    int selectedFile;
                    cin >> selectedFile;

                    string filePath = folderPath + "/" + tentep[selectedFile - 1];

                    if (filesystem::exists(filePath)) {
                        displayFileInfo(filePath);
                    }
                    else {
                        cout << "File not found." << endl;
                    }
                }
                if (LUA_CHON1 == 3) {
                    string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all/quantrong";
                    vector<string> txtFiles = listTxtFiles(folderPath);
                    cout << "Total number of txt files: " << txtFiles.size() << endl;
                    cout << "List of txt files:" << endl;

                    vector<string> tentep;
                    for (const auto& fileName : txtFiles) {

                        cout << fileName << endl;
                        tentep.push_back(fileName);
                    }

                    cout << "Enter the number of file txt you want to view: ";
                    int selectedFile;
                    cin >> selectedFile;

                    string filePath = folderPath + "/" + tentep[selectedFile - 1];

                    if (filesystem::exists(filePath)) {
                        displayFileInfo(filePath);
                    }
                    else {
                        cout << "File not found." << endl;
                    }
                }
                if (LUA_CHON1 == 4) {
                    string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all/vieclam";
                    vector<string> txtFiles = listTxtFiles(folderPath);
                    cout << "Total number of txt files: " << txtFiles.size() << endl;
                    cout << "List of txt files:" << endl;

                    vector<string> tentep;
                    for (const auto& fileName : txtFiles) {

                        cout << fileName << endl;
                        tentep.push_back(fileName);
                    }

                    cout << "Enter the number of file txt you want to view: ";
                    int selectedFile;
                    cin >> selectedFile;

                    string filePath = folderPath + "/" + tentep[selectedFile - 1];

                    if (filesystem::exists(filePath)) {
                        displayFileInfo(filePath);
                    }
                    else {
                        cout << "File not found." << endl;
                    }
                }
                if (LUA_CHON1 == 5) {
                    string folderPath = "C:/Users/ADMIN/source/repos/socketClient/socketClient/inbox/all/binhthuong";
                    vector<string> txtFiles = listTxtFiles(folderPath);
                    cout << "Total number of txt files: " << txtFiles.size() << endl;
                    cout << "List of txt files:" << endl;

                    vector<string> tentep;
                    for (const auto& fileName : txtFiles) {

                        cout << fileName << endl;
                        tentep.push_back(fileName);
                    }

                    cout << "Enter the number of file txt you want to view: ";
                    int selectedFile;
                    cin >> selectedFile;

                    string filePath = folderPath + "/" + tentep[selectedFile - 1];

                    if (filesystem::exists(filePath)) {
                        displayFileInfo(filePath);
                    }
                    else {
                        cout << "File not found." << endl;
                    }
                }
            }
        }

        if (mn == 4) {

            int i = 0;
            int s = 0;

            cout << "\nBan da chon: Tai email tu dong.";
            cout << "\nNhap so giay tai tu dong ma ban muon: "; cin >> s;

            string ten_nguoi_dung, mat_khau;
            while (true) {
                if (_kbhit()) {
                    char key = _getch();
                    std::cout << "Key pressed: " << key << std::endl;

                    if (key == 27) {  // Mã ASCII của phím Esc là 27
                        std::cout << "Esc key pressed. Exiting..." << std::endl;
                        break;  // Thoát khỏi vòng lặp khi phím Esc được nhấn
                    }
                }
                // Kết nối máy chủ
                if (!khoi_tao_winsock()) {
                    cerr << "Khong the khoi tao Winsock." << endl;
                    return 1;
                }
                const char* pop3 = "127.0.0.1";
                int POP3 = 578;
                sockaddr_in thongTinServerPOP3 = thiet_lap_thong_tin_server_POP3(pop3, POP3);
                SOCKET socketClientPOP3 = tao_socket();
                if (socketClientPOP3 == INVALID_SOCKET)
                {
                    cerr << "Khong the tao socket." << endl;
                    dong_ket_noi_va_don_sach_winsock(socketClientPOP3);
                    return 1;
                }

                if (!ket_noi_den_server(socketClientPOP3, thongTinServerPOP3)) {
                    cerr << "Khong the ket noi den server POP3." << endl;
                    dong_ket_noi_va_don_sach_winsock(socketClientPOP3);
                    return 0;
                }
                else
                    cerr << "Da ket noi den server POP3." << endl;
                if (!ket_noi_den_server_POP3(socketClientPOP3, thongTinServerPOP3)) {
                    cerr << "Khong the ket noi den server POP3." << endl;
                    dong_ket_noi_va_don_sach_winsock_POP3(socketClientPOP3);
                    return 1;
                }
                if (i == 0) {
                    //Nhập thông tin đăng nhập
                    cout << "\nDang nhap Tai Khoan";
                    cout << "\nTen Tai Khoan: "; cin >> ten_nguoi_dung;
                    cout << "\nMat Khau: "; cin >> mat_khau;
                    i++;
                }
                // Đăng nhập
                if (!dang_nhap_POP3(socketClientPOP3, ten_nguoi_dung, mat_khau)) {
                    cerr << "Dang nhap that bai." << endl;
                    dong_ket_noi_va_don_sach_winsock_POP3(socketClientPOP3);
                    return 1;
                }
                int so_luong_email = 0;
                lay_danh_sach_email_POP3(socketClientPOP3, so_luong_email);

                tai_toan_bo_email_POP3(socketClientPOP3, so_luong_email);

                dong_ket_noi_va_don_sach_winsock_POP3(socketClientPOP3);
                // Chờ s giây giữa hai lần gửi để giả định là có một khoảng thời gian giữa các tác vụ
                this_thread::sleep_for(chrono::seconds(s));
            }
        }

        if (mn == 5)  return 0;
    }

}

