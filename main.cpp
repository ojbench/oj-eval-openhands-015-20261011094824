#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

const std::string DATA_FILE = "database.dat";

class FileDatabase {
private:
    std::string data_filename;
    std::unordered_map<std::string, std::set<int>> data; // Keep index in memory
    
    bool fileExists(const std::string& filename) {
        return fs::exists(filename);
    }
    
    void loadFromFile() {
        if (!fileExists(data_filename)) return;
        
        std::ifstream file(data_filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string key;
            int value;
            
            if (iss >> key >> value) {
                data[key].insert(value);
            }
        }
        file.close();
    }
    
    void saveToFile() {
        std::ofstream file(data_filename);
        
        for (const auto& pair : data) {
            for (int value : pair.second) {
                file << pair.first << " " << value << "\n";
            }
        }
        file.close();
    }
    
public:
    FileDatabase(const std::string& fname) : data_filename(fname) {
        loadFromFile(); // Load data at startup
    }
    
    ~FileDatabase() {
        saveToFile(); // Save data at shutdown
    }
    
    void insert(const std::string& key, int value) {
        // Check if entry already exists
        if (data[key].find(value) != data[key].end()) {
            return;
        }
        
        data[key].insert(value);
    }
    
    void deleteEntry(const std::string& key, int value) {
        auto it = data.find(key);
        if (it == data.end()) return;
        
        it->second.erase(value);
        
        // Remove empty key
        if (it->second.empty()) {
            data.erase(it);
        }
    }
    
    void find(const std::string& key) {
        auto it = data.find(key);
        if (it == data.end() || it->second.empty()) {
            std::cout << "null\n";
            return;
        }
        
        // Values are already sorted in set
        bool first = true;
        for (int value : it->second) {
            if (!first) std::cout << " ";
            std::cout << value;
            first = false;
        }
        std::cout << "\n";
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    FileDatabase db(DATA_FILE);
    
    int n;
    std::cin >> n;
    std::cin.ignore(); // Ignore newline after n
    
    for (int i = 0; i < n; ++i) {
        std::string line;
        std::getline(std::cin, line);
        
        std::istringstream iss(line);
        std::string command;
        iss >> command;
        
        if (command == "insert") {
            std::string index;
            int value;
            iss >> index >> value;
            db.insert(index, value);
        } else if (command == "delete") {
            std::string index;
            int value;
            iss >> index >> value;
            db.deleteEntry(index, value);
        } else if (command == "find") {
            std::string index;
            iss >> index;
            db.find(index);
        }
    }
    
    return 0;
}