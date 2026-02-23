#include "bits/stdc++.h"

using namespace std;

// Base Component
class Node
{
public:
    string name;
    bool isDirectory;
    virtual ~Node() = default;
    Node(string n, bool dir) : name(n), isDirectory(dir) {}
};

// Leaf Node
class File : public Node
{
public:
    string content;
    File(string n, string c = "") : Node(n, false), content(c) {}
};

// Composite Node
class Directory : public Node
{
public:
    map<string, shared_ptr<Node>> children;
    weak_ptr<Directory> parent; // Prevents memory leaks

    Directory(string n, shared_ptr<Directory> p = nullptr) : Node(n, true), parent(p) {}

    void addNode(shared_ptr<Node> node)
    {
        children[node->name] = node;
    }
};

class FileSystem
{
private:
    shared_ptr<Directory> root;
    shared_ptr<Directory> cwd;

    // Helper to split path strings
    vector<string> parse(string path)
    {
        vector<string> parts;
        stringstream ss(path);
        string item;
        while (getline(ss, item, '/'))
        {
            if (!item.empty())
                parts.push_back(item);
        }
        return parts;
    }

    shared_ptr<Directory> resolveRecursive(shared_ptr<Directory> current, const vector<string> &parts, int index)
    {
        if (index == parts.size())
            return current;

        string pattern = parts[index];

        // Handle ".." specifically
        if (pattern == "..")
        {
            auto parent = current->parent.lock();
            return resolveRecursive(parent ? parent : root, parts, index + 1);
        }
        // Handle "."
        if (pattern == ".")
        {
            return resolveRecursive(current, parts, index + 1);
        }

        // Convert Linux wildcard '*' to Regex '.*'
        // If the user didn't provide a regex, we escape special chars,
        // but for this exercise, let's treat the input as a potential regex.
        string regexPattern = "";
        for (char c : pattern)
        {
            if (c == '*')
                regexPattern += ".*";
            else
                regexPattern += c;
        }

        try
        {
            regex r(regexPattern);
            for (auto it : current->children)
            {
                if (it.second->isDirectory && regex_match(it.first, r))
                {
                    // Recurse to see if this branch matches the rest of the path
                    auto found = resolveRecursive(dynamic_pointer_cast<Directory>(it.second), parts, index + 1);
                    if (found)
                        return found;
                }
            }
        }
        catch (const regex_error &e)
        {
            return nullptr;
        }

        return nullptr;
    }

public:
    FileSystem()
    {
        root = make_shared<Directory>("/");
        cwd = root;
    }

    void mkdir(string path)
    {
        auto parts = parse(path);
        shared_ptr<Directory> curr = (path[0] == '/') ? root : cwd;

        for (const string &part : parts)
        {
            if (curr->children.find(part) == curr->children.end())
            {
                auto newDir = make_shared<Directory>(part, curr);
                curr->addNode(newDir);
                curr = newDir;
            }
            else
            {
                curr = dynamic_pointer_cast<Directory>(curr->children[part]);
            }
        }
    }

    string pwd()
    {
        if (cwd == root)
            return "/";

        string path = "";
        shared_ptr<Directory> temp = cwd;

        while (temp != nullptr && temp != root)
        {
            path = "/" + temp->name + path;
            temp = temp->parent.lock(); // Use .lock() to get shared_ptr from weak_ptr
        }

        return path.empty() ? "/" : path;
    }

    void cd(string path)
    {
        if (path == "/")
        {
            cwd = root;
            return;
        }

        vector<string> parts = parse(path);
        shared_ptr<Directory> searchStart = (path[0] == '/') ? root : cwd;

        shared_ptr<Directory> result = resolveRecursive(searchStart, parts, 0);

        if (result)
        {
            cwd = result;
        }
        else
        {
            cout << "cd: no such file or directory: " << path << endl;
        }
    }

    void ls()
    {
        for (auto it : cwd->children)
        {
            cout << it.first << (it.second->isDirectory ? "/" : "") << "  ";
        }
        cout << endl;
    }
};

int main()
{
    FileSystem fs;
    fs.mkdir("/home/user/documents");
    fs.mkdir("/home/user/downloads");
    fs.mkdir("/home/admin/logs");

    fs.cd("/home/*/doc*"); // Matches /home/user/documents
    cout << "Current Path: " << fs.pwd() << endl;

    fs.cd("..");
    cout << "After 'cd ..': " << fs.pwd() << endl;

    fs.cd("/home/admin/l.gs"); // Regex match for 'logs'
    cout << "Regex match: " << fs.pwd() << endl;
}