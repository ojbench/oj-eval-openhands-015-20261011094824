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
const std::string INDEX_FILE = "database.idx";

struct IndexEntry {
    long file_pos;
    int value;
    bool deleted;
    
    IndexEntry(long pos, int val, bool del = false) 
        : file_pos(pos), value(val), deleted(del) {}
};

class FileDatabase {
private:
    std::string data_filename;
    std::string index_filename;
    
    bool fileExists(const std::string& filename) {
        return fs::exists(filename);
    }
    
    void loadIndex(std::unordered_map<std::string, std::vector<IndexEntry>>& index) {
        if (!fileExists(index_filename)) return;
        
        std::ifstream file(index_filename, std::ios::binary);
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::istringstream iss(line);
            std::string key;
            long file_pos;
            int value;
            char deleted;
            
            if (iss >> key >> file_pos >> value >> deleted) {
                index[key].emplace_back(file_pos, value, deleted == '1');
            }
        }
        file.close();
    }
    
    void saveIndex(const std::unordered_map<std::string, std::vector<IndexEntry>>& index) {
        std::ofstream file(index_filename);
        
        for (const auto& pair : index) {
            for (const auto& entry : pair.second) {
                file << pair.first << " " << entry.file_pos << " " 
                     << entry.value << " " << (entry.deleted ? '1' : '0') << "\n";
            }
        }
        file.close();
    }
    
    bool entryExistsInIndex(const std::unordered_map<std::string, std::vector<IndexEntry>>& index,
                           const std::string& key, int value) {
        auto it = index.find(key);
        if (it == index.end()) return false;
        
        for (const auto& entry : it->second) {
            if (!entry.deleted && entry.value == value) {
                return true;
            }
        }
        return false;
    }
    
public:
    FileDatabase(const std::string& data_fname, const std::string& idx_fname) 
        : data_filename(data_fname), index_filename(idx_fname) {}
    
    void insert(const std::string& key, int value) {
        // Load index
        std::unordered_map<std::string, std::vector<IndexEntry>> index;
        loadIndex(index);
        
        // Check if entry already exists
        if (entryExistsInIndex(index, key, value)) {
            return;
        }
        
        // Append to data file
        std::ofstream data_file(data_filename, std::ios::app);
        long file_pos = data_file.tellp();
        data_file << key << " " << value << "\n";
        data_file.close();
        
        // Update index
        index[key].emplace_back(file_pos, value, false);
        saveIndex(index);
    }
    
    void deleteEntry(const std::string& key, int value) {
        // Load index
        std::unordered_map<std::string, std::vector<IndexEntry>> index;
        loadIndex(index);
        
        // Find and mark as deleted
        auto it = index.find(key);
        if (it == index.end()) return;
        
        bool found = false;
        for (auto& entry : it->second) {
            if (!entry.deleted && entry.value == value) {
                entry.deleted = true;
                found = true;
                break;
            }
        }
        
        if (found) {
            saveIndex(index);
        }
    }
    
    void find(const std::string& key) {
        // Load index
        std::unordered_map<std::string, std::vector<IndexEntry>> index;
        loadIndex(index);
        
        auto it = index.find(key);
        if (it == index.end()) {
            std::cout << "null\n";
            return;
        }
        
        std::vector<int> values;
        for (const auto& entry : it->second) {
            if (!entry.deleted) {
                values.push_back(entry.value);
            }
        }
        
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