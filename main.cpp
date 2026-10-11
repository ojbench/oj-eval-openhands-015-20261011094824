#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

const std::string DATA_FILE = "database.dat";

struct Entry {
    std::string index;
    int value;
    bool deleted;
    
    Entry(const std::string& idx, int val, bool del = false) 
        : index(idx), value(val), deleted(del) {}
};

class FileDatabase {
private:
    std::string filename;
    
    bool fileExists() {
        return fs::exists(filename);
    }
    
    bool entryExists(const std::string& index, int value) {
        if (!fileExists()) return false;
        
        std::ifstream file(filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            char status;
            std::string fileIndex;
            int fileValue;
            
            if (iss >> status >> fileIndex >> fileValue) {
                if (status == 'A' && fileIndex == index && fileValue == value) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
public:
    FileDatabase(const std::string& fname) : filename(fname) {}
    
    void insert(const std::string& index, int value) {
        // Check if entry already exists
        if (entryExists(index, value)) {
            return;
        }
        
        std::ofstream file(filename, std::ios::app);
        file << "A " << index << " " << value << "\n";
        file.close();
    }
    
    void deleteEntry(const std::string& index, int value) {
        if (!fileExists()) return;
        
        // Read all entries
        std::vector<std::string> lines;
        std::ifstream file(filename);
        std::string line;
        bool found = false;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            char status;
            std::string fileIndex;
            int fileValue;
            
            if (iss >> status >> fileIndex >> fileValue) {
                if (status == 'A' && fileIndex == index && fileValue == value) {
                    // Mark as deleted
                    lines.push_back("D " + fileIndex + " " + std::to_string(fileValue));
                    found = true;
                } else {
                    lines.push_back(line);
                }
            }
        }
        file.close();
        
        // Write back all entries
        if (found) {
            std::ofstream outFile(filename);
            for (const auto& l : lines) {
                outFile << l << "\n";
            }
            outFile.close();
        }
    }
    
    void find(const std::string& index) {
        if (!fileExists()) {
            std::cout << "null\n";
            return;
        }
        
        std::vector<int> values;
        std::ifstream file(filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            char status;
            std::string fileIndex;
            int fileValue;
            
            if (iss >> status >> fileIndex >> fileValue) {
                if (status == 'A' && fileIndex == index) {
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