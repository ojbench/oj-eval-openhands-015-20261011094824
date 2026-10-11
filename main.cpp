#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

const std::string DATA_FILE = "database.dat";

class FileDatabase {
private:
    std::string data_filename;
    
    bool fileExists(const std::string& filename) {
        return fs::exists(filename);
    }
    
    bool entryExists(const std::string& key, int value) {
        if (!fileExists(data_filename)) return false;
        
        std::ifstream file(data_filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                if (fileKey == key && fileValue == value) {
                    file.close();
                    return true;
                }
            }
        }
        file.close();
        return false;
    }
    
    void deleteFromFile(const std::string& key, int value) {
        if (!fileExists(data_filename)) return;
        
        std::vector<std::string> lines;
        std::ifstream file(data_filename);
        std::string line;
        bool found = false;
        
        while (std::getline(file, line)) {
            if (line.empty()) {
                lines.push_back(line);
                continue;
            }
            
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                if (fileKey == key && fileValue == value) {
                    found = true;
                    // Skip this line (delete it)
                } else {
                    lines.push_back(line);
                }
            } else {
                lines.push_back(line);
            }
        }
        file.close();
        
        if (found) {
            std::ofstream outFile(data_filename);
            for (const auto& l : lines) {
                outFile << l << "\n";
            }
            outFile.close();
        }
    }
    
public:
    FileDatabase(const std::string& fname) : data_filename(fname) {}
    
    void insert(const std::string& key, int value) {
        // Check if entry already exists
        if (entryExists(key, value)) {
            return;
        }
        
        // Append to file
        std::ofstream file(data_filename, std::ios::app);
        file << key << " " << value << "\n";
        file.close();
    }
    
    void deleteEntry(const std::string& key, int value) {
        deleteFromFile(key, value);
    }
    
    void find(const std::string& key) {
        if (!fileExists(data_filename)) {
            std::cout << "null\n";
            return;
        }
        
        std::vector<int> values;
        std::ifstream file(data_filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                if (fileKey == key) {
                    values.push_back(fileValue);
                }
            }
        }
        file.close();
        
        if (values.empty()) {
            std::cout << "null\n";
        } else {
            std::sort(values.begin(), values.end());
            for (size_t i = 0; i < values.size(); ++i) {
                if (i > 0) std::cout << " ";
                std::cout << values[i];
            }
            std::cout << "\n";
        }
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