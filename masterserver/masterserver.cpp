#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>

using namespace std;

struct ServerInfo {
    string ip;
    string filename;
    vector<uint8_t> data;
    chrono::steady_clock::time_point last_seen;
};

mutex g_mutex;
map<string, ServerInfo> g_servers; // key is "ip:filename"

void handle_client(int client_sock, sockaddr_in client_addr) {
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    string client_ip = ip_str;

    vector<uint8_t> buffer;
    char chunk[4096];
    
    // Read HTTP headers
    bool headers_done = false;
    int content_length = 0;
    while (!headers_done) {
        int n = recv(client_sock, chunk, sizeof(chunk), 0);
        if (n <= 0) break;
        buffer.insert(buffer.end(), chunk, chunk + n);
        
        string req(buffer.begin(), buffer.end());
        auto pos = req.find("\r\n\r\n");
        if (pos != string::npos) {
            headers_done = true;
            // parse Content-Length
            auto cl_pos = req.find("Content-Length: ");
            if (cl_pos == string::npos) {
                cl_pos = req.find("Content-length: ");
            }
            if (cl_pos != string::npos) {
                cl_pos += 16;
                auto cl_end = req.find("\r\n", cl_pos);
                if (cl_end != string::npos) {
                    content_length = stoi(req.substr(cl_pos, cl_end - cl_pos));
                }
            }
        }
    }
    
    if (!headers_done) {
        close(client_sock);
        return;
    }
    
    string req(buffer.begin(), buffer.end());
    auto header_end = req.find("\r\n\r\n") + 4;
    
    // Read rest of body if needed
    int body_received = buffer.size() - header_end;
    while (body_received < content_length) {
        int n = recv(client_sock, chunk, sizeof(chunk), 0);
        if (n <= 0) break;
        buffer.insert(buffer.end(), chunk, chunk + n);
        body_received += n;
    }
    
    req = string(buffer.begin(), buffer.end());
    
    string method = req.substr(0, req.find(" "));
    auto path_end = req.find(" ", method.length() + 1);
    string path = req.substr(method.length() + 1, path_end - method.length() - 1);
    
    if (method == "GET") {
        if (path == "/" || path == "/cgi-bin/cpmaster.pl") {
            // Serve HTML list
            stringstream html;
            html << "<html><head><title>Clonk Planet Master Server</title></head><body>";
            html << "<h1>Clonk Planet Master Server</h1>";
            html << "<h2>Active Games</h2><ul>";
            
            lock_guard<mutex> lock(g_mutex);
            auto now = chrono::steady_clock::now();
            for (auto it = g_servers.begin(); it != g_servers.end(); ) {
                if (chrono::duration_cast<chrono::seconds>(now - it->second.last_seen).count() > 150) {
                    it = g_servers.erase(it); // Timeout
                } else {
                    html << "<li><a href=\"/file/" << it->second.ip << "/" << it->second.filename << "\">" 
                         << it->second.ip << " - " << it->second.filename << "</a></li>";
                    ++it;
                }
            }
            html << "</ul></body></html>";
            
            string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(html.str().length()) + "\r\n\r\n" + html.str();
            send(client_sock, response.c_str(), response.length(), 0);
        } else if (path.find("/file/") == 0) {
            // Serve file
            string req_path = path.substr(6);
            auto slash = req_path.find("/");
            if (slash != string::npos) {
                string ip = req_path.substr(0, slash);
                string filename = req_path.substr(slash + 1);
                string key = ip + ":" + filename;
                
                lock_guard<mutex> lock(g_mutex);
                if (g_servers.count(key)) {
                    vector<uint8_t>& data = g_servers[key].data;
                    string response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + to_string(data.size()) + "\r\nContent-Disposition: attachment; filename=\"" + filename + "\"\r\n\r\n";
                    send(client_sock, response.c_str(), response.length(), 0);
                    send(client_sock, data.data(), data.size(), 0);
                } else {
                    string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                    send(client_sock, response.c_str(), response.length(), 0);
                }
            }
        } else {
            string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(client_sock, response.c_str(), response.length(), 0);
        }
    } else if (method == "POST") {
        // Parse POST body (form urlencoded prefix)
        size_t text_end = req.find("\r\n", header_end);
        if (text_end != string::npos) {
            string post_str = req.substr(header_end, text_end - header_end);
            
            // Extract action and filename
            string action = "";
            string filename = "";
            
            size_t action_pos = post_str.find("action=");
            if (action_pos != string::npos) {
                size_t amp = post_str.find("&", action_pos);
                if (amp == string::npos) amp = post_str.length();
                action = post_str.substr(action_pos + 7, amp - (action_pos + 7));
            }
            
            size_t file_pos = post_str.find("filename=");
            if (file_pos != string::npos) {
                size_t amp = post_str.find("&", file_pos);
                if (amp == string::npos) amp = post_str.length();
                filename = post_str.substr(file_pos + 9, amp - (file_pos + 9));
            }
            
            string key = client_ip + ":" + filename;
            
            lock_guard<mutex> lock(g_mutex);
            if (action == "upload") {
                size_t binary_start = text_end + 2;
                vector<uint8_t> data;
                if (buffer.size() > binary_start) {
                    data = vector<uint8_t>(buffer.begin() + binary_start, buffer.end());
                }
                g_servers[key] = {client_ip, filename, data, chrono::steady_clock::now()};
                cout << "Registered game: " << key << " (" << data.size() << " bytes)" << endl;
            } else if (action == "keep") {
                if (g_servers.count(key)) {
                    g_servers[key].last_seen = chrono::steady_clock::now();
                    cout << "Kept alive: " << key << endl;
                }
            } else if (action == "delete") {
                g_servers.erase(key);
                cout << "Deleted: " << key << endl;
            }
            
            string response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
            send(client_sock, response.c_str(), response.length(), 0);
        }
    }
    
    close(client_sock);
}

int main(int argc, char** argv) {
    int port = 8080;
    if (argc > 1) {
        port = stoi(argv[1]);
    }
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Failed to bind to port " << port << endl;
        return 1;
    }
    
    if (listen(server_sock, 10) < 0) {
        cerr << "Failed to listen" << endl;
        return 1;
    }
    
    cout << "Master server listening on port " << port << "..." << endl;
    
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);
        if (client_sock >= 0) {
            thread(handle_client, client_sock, client_addr).detach();
        }
    }
    
    return 0;
}
