#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;

const std::string DATA_FILE = "database.dat";
const std::string INDEX_FILE = "database.idx";

class FileDatabase {
private:
    std::string data_filename;
    std::string index_filename;
    std::unordered_map<std::string, std::vector<long>> key_positions; // Cache key positions
    
    bool fileExists(const std::string& filename) {
        return fs::exists(filename);
    }
    
    void loadIndex() {
        key_positions.clear();
        if (!fileExists(index_filename)) return;
        
        std::ifstream file(index_filename);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string key;
            long pos;
            
            if (iss >> key >> pos) {
                key_positions[key].push_back(pos);
            }
        }
        file.close();
    }
    
    void saveIndex() {
        std::ofstream file(index_filename);
        
        for (const auto& pair : key_positions) {
            for (long pos : pair.second) {
                file << pair.first << " " << pos << "\n";
            }
        }
        file.close();
    }
    
    bool entryExistsAtPosition(long pos, const std::string& key, int value) {
        std::ifstream file(data_filename);
        file.seekg(pos);
        
        std::string line;
        if (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                file.close();
                return fileKey == key && fileValue == value;
            }
        }
        file.close();
        return false;
    }
    
    bool entryExists(const std::string& key, int value) {
        auto it = key_positions.find(key);
        if (it != key_positions.end()) {
            // Check cached positions
            for (long pos : it->second) {
                if (entryExistsAtPosition(pos, key, value)) {
                    return true;
                }
            }
        }
        
        // Full scan if not found in cache
        if (!fileExists(data_filename)) return false;
        
        std::ifstream file(data_filename);
        std::string line;
        long current_pos = 0;
        
        while (std::getline(file, line)) {
            if (line.empty()) {
                current_pos = file.tellg();
                continue;
            }
            
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                if (fileKey == key && fileValue == value) {
                    // Update cache
                    key_positions[key].push_back(current_pos);
                    file.close();
                    return true;
                }
                if (fileKey == key) {
                    // Cache position for this key
                    key_positions[key].push_back(current_pos);
                }
            }
            current_pos = file.tellg();
        }
        file.close();
        return false;
    }
    
public:
    FileDatabase(const std::string& data_fname, const std::string& idx_fname) 
        : data_filename(data_fname), index_filename(idx_fname) {
        loadIndex();
    }
    
    ~FileDatabase() {
        saveIndex();
    }
    
    void insert(const std::string& key, int value) {
        // Check if entry already exists
        if (entryExists(key, value)) {
            return;
        }
        
        // Append to file
        std::ofstream file(data_filename, std::ios::app);
        long pos = file.tellp();
        file << key << " " << value << "\n";
        file.close();
        
        // Update index
        key_positions[key].push_back(pos);
    }
    
    void deleteEntry(const std::string& key, int value) {
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
                    // Remove from index
                    auto it = key_positions.find(key);
                    if (it != key_positions.end()) {
                        it->second.clear();
                    }
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
            long current_pos = 0;
            for (const auto& l : lines) {
                if (!l.empty()) {
                    std::istringstream iss(l);
                    std::string fileKey;
                    int fileValue;
                    if (iss >> fileKey >> fileValue) {
                        key_positions[fileKey].push_back(current_pos);
                    }
                }
                outFile << l << "\n";
                current_pos = outFile.tellp();
            }
            outFile.close();
        }
    }
    
    void find(const std::string& key) {
        if (!fileExists(data_filename)) {
            std::cout << "null\n";
            return;
        }
        
        std::vector<int> values;
        std::unordered_set<int> seen; // Avoid duplicates
        
        // Always do full scan to ensure correctness
        std::ifstream file(data_filename);
        std::string line;
        long current_pos = 0;
        
        while (std::getline(file, line)) {
            if (line.empty()) {
                current_pos = file.tellg();
                continue;
            }
            
            std::istringstream iss(line);
            std::string fileKey;
            int fileValue;
            
            if (iss >> fileKey >> fileValue) {
                if (fileKey == key && seen.find(fileValue) == seen.end()) {
                    values.push_back(fileValue);
                    seen.insert(fileValue);
                    // Cache position for this key
                    key_positions[key].push_back(current_pos);
                }
            }
            current_pos = file.tellg();
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
    
    FileDatabase db(DATA_FILE, INDEX_FILE);
    
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