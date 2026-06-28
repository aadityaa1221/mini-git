#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include "sha1.hpp" // Pulls in your new cryptographic engine

namespace fs = std::filesystem;
using namespace std;

// ==========================================
// PHASE 1: INITIALIZE REPOSITORY
// ==========================================
void initRepository()
{
    string gitDir = ".mygit";

    if (fs::exists(gitDir))
    {
        cout << "Mini-Git repository already exists.\n";
        return;
    }

    try
    {
        fs::create_directory(gitDir);
        fs::create_directory(gitDir + "/objects");
        fs::create_directory(gitDir + "/refs");

        ofstream headFile(gitDir + "/HEAD");
        if (headFile.is_open())
        {
            headFile << "ref: refs/heads/main\n";
            headFile.close();
        }
        cout << "Initialized empty Mini-Git repository in .mygit/\n";
    }
    catch (const fs::filesystem_error &e)
    {
        cerr << "File system error: " << e.what() << '\n';
    }
}

// ==========================================
// PHASE 2: HASH AND STORE BLOB
// ==========================================
void addFile(const string &filepath)
{
    // 1. Edge Case: Check if file exists
    if (!fs::exists(filepath))
    {
        cerr << "Fatal: pathspec '" << filepath << "' did not match any files.\n";
        return;
    }
    if (!fs::exists(".mygit"))
    {
        cerr << "Fatal: Not a mygit repository. Run 'mygit init' first.\n";
        return;
    }

    // 2. Read the raw file bytes
    ifstream file(filepath, ios::binary);
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // 3. Construct the exact Git Header (blob <size>\0<content>)
    string store = "blob " + to_string(content.size()) + '\0' + content;

    // 4. Generate the SHA-1 Cryptographic Hash
    SHA1 sha1;
    sha1.update(store);
    string hash = sha1.final();

    // 5. Save the Blob inside our hidden objects folder using the hash as the filename
    string objectPath = ".mygit/objects/" + hash;
    ofstream outFile(objectPath, ios::binary);
    outFile << store;
    outFile.close();

    cout << "Added '" << filepath << "' to storage.\n";
    cout << "Generated Blob Hash: " << hash << "\n";
}

// ==========================================
// PHASE 3: THE SNAPSHOT (COMMIT)
// ==========================================
void commit(const string &message)
{
    if (!fs::exists(".mygit"))
    {
        cerr << "Fatal: Not a mygit repository.\n";
        return;
    }

    // 1. Build the Tree Object
    string treeContent = "";

    // For this MVP, we will automatically track all .txt files in the current folder
    for (const auto &entry : fs::directory_iterator("."))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            string filename = entry.path().filename().string();

            // Read and hash the file (simulating the 'add' process for the tree)
            ifstream file(entry.path(), ios::binary);
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
            file.close();

            string store = "blob " + to_string(content.size()) + '\0' + content;
            SHA1 sha1;
            sha1.update(store);
            string hash = sha1.final();

            // Append to tree format: <mode> blob <hash>\t<filename>\n
            treeContent += "100644 blob " + hash + "\t" + filename + "\n";
        }
    }

    if (treeContent.empty())
    {
        cout << "Nothing to commit (no .txt files found).\n";
        return;
    }

    // Save the Tree Object
    string treeStore = "tree " + to_string(treeContent.size()) + '\0' + treeContent;
    SHA1 treeSha1;
    treeSha1.update(treeStore);
    string treeHash = treeSha1.final();

    ofstream treeFile(".mygit/objects/" + treeHash, ios::binary);
    treeFile << treeStore;
    treeFile.close();

    // 2. Build the Commit Object
    string commitContent = "tree " + treeHash + "\n";
    commitContent += "author SDE <sde@minigit.local>\n\n";
    commitContent += message + "\n";

    string commitStore = "commit " + to_string(commitContent.size()) + '\0' + commitContent;
    SHA1 commitSha1;
    commitSha1.update(commitStore);
    string commitHash = commitSha1.final();

    ofstream commitFile(".mygit/objects/" + commitHash, ios::binary);
    commitFile << commitStore;
    commitFile.close();

    // 3. Update the main branch reference
    ofstream refFile(".mygit/refs/heads/main");
    refFile << commitHash << "\n";
    refFile.close();

    // Print success message mimicking real Git
    cout << "[main " << commitHash.substr(0, 7) << "] " << message << "\n";
}

// ==========================================
// MAIN CLI ROUTER
// ==========================================
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: ./mygit <command>\n";
        cerr << "Commands: init, add <file>, commit <message>\n";
        return 1;
    }

    string command = argv[1];

    if (command == "init")
    {
        initRepository();
    }
    else if (command == "add")
    {
        if (argc < 3)
        {
            cerr << "Usage: ./mygit add <file>\n";
            return 1;
        }
        addFile(argv[2]);
    }
    else if (command == "commit")
    {
        if (argc < 3)
        {
            cerr << "Usage: ./mygit commit \"<message>\"\n";
            return 1;
        }
        commit(argv[2]);
    }
    else
    {
        cerr << "mygit: '" << command << "' is not a mygit command.\n";
        return 1;
    }

    return 0;
}