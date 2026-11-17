

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
using namespace std;

const int SUBJECTS = 3;
const int PASS_MARK = 40;


class InvalidMarksException : public runtime_error {
public:
    InvalidMarksException(int m)
        : runtime_error("Invalid marks entered: " + to_string(m) +
                        " (must be between 0 and 100)") {}
};


class Student {
protected:
    string name;
    string usn;
public:
    Student(const string& n, const string& u) : name(n), usn(u) {}
    virtual ~Student() {}

    virtual void displayDetails() const {
        cout << "Name: " << name << "  |  USN: " << usn << endl;
    }
    virtual void displayMarks() const = 0;      
    virtual int  getTotal() const = 0;
    virtual string toFileLine() const = 0;
};


class Result : public Student {
    int marks[SUBJECTS];
public:
    Result(const string& n, const string& u, const int m[SUBJECTS])
        : Student(n, u) {
        for (int i = 0; i < SUBJECTS; i++) {
            if (m[i] < 0 || m[i] > 100)
                throw InvalidMarksException(m[i]);
            marks[i] = m[i];
        }
    }

    int getTotal() const override {
        int t = 0;
        for (int i = 0; i < SUBJECTS; i++) t += marks[i];
        return t;
    }

    bool hasPassed() const {
        for (int i = 0; i < SUBJECTS; i++)
            if (marks[i] < PASS_MARK) return false;
        return true;
    }

    void displayMarks() const override {
        cout << "Marks: ";
        for (int i = 0; i < SUBJECTS; i++) cout << marks[i] << " ";
        cout << "| Total: " << getTotal() << "/" << SUBJECTS * 100
             << " | Result: " << (hasPassed() ? "PASS" : "FAIL") << endl;
    }

    string toFileLine() const override {
        ostringstream os;
        os << name << "|" << usn;
        for (int i = 0; i < SUBJECTS; i++) os << "|" << marks[i];
        return os.str();
    }
};

typedef vector<unique_ptr<Student>> StudentList;


void addStudent(StudentList& list) {
    string name, usn;
    int m[SUBJECTS];
    cin.ignore(1000, '\n');
    cout << "Enter name: ";
    getline(cin, name);
    cout << "Enter USN: ";
    getline(cin, usn);
    for (int i = 0; i < SUBJECTS; i++) {
        cout << "Enter marks for subject " << i + 1 << " (0-100): ";
        cin >> m[i];
    }
    try {
        list.push_back(make_unique<Result>(name, usn, m));
        cout << "Record added successfully.\n";
    } catch (const InvalidMarksException& e) {
        cout << "Error: " << e.what() << "\nRecord NOT added.\n";
    }
}

void displayAll(const StudentList& list) {
    if (list.empty()) { cout << "No records available.\n"; return; }
    cout << "\n--- Student Records ---\n";
    for (const auto& s : list) {
        s->displayDetails();   
        s->displayMarks();
        cout << "------------------------\n";
    }
}

void saveToFile(const StudentList& list, const string& fname) {
    ofstream out(fname);
    if (!out) { cout << "Could not open file for writing.\n"; return; }
    for (const auto& s : list) out << s->toFileLine() << "\n";
    cout << list.size() << " record(s) saved to " << fname << endl;
}

void loadFromFile(StudentList& list, const string& fname) {
    ifstream in(fname);
    if (!in) { cout << "File not found: " << fname << endl; return; }
    list.clear();
    string line;
    int count = 0;
    while (getline(in, line)) {
        stringstream ss(line);
        string name, usn, tok;
        int m[SUBJECTS];
        getline(ss, name, '|');
        getline(ss, usn, '|');
        try {
            for (int i = 0; i < SUBJECTS; i++) {
                getline(ss, tok, '|');
                m[i] = stoi(tok);
            }
            list.push_back(make_unique<Result>(name, usn, m));
            count++;
        } catch (const exception& e) {
            cout << "Skipping bad line: " << line << " (" << e.what() << ")\n";
        }
    }
    cout << count << " record(s) loaded from " << fname << endl;
}

void showAbove(const StudentList& list) {
    int limit;
    cout << "Show students with total above: ";
    cin >> limit;

    
    StudentList::const_iterator it = list.begin();
    int found = 0;
    cout << "\nStudents scoring above " << limit << ":\n";
    for (const auto& s : list) {
        auto above = [limit](const unique_ptr<Student>& p) {
            return p->getTotal() > limit;
        };
        if (above(s)) { s->displayDetails(); s->displayMarks(); found++; }
    }
    int cnt = count_if(list.begin(), list.end(),
        [limit](const unique_ptr<Student>& p) { return p->getTotal() > limit; });
    cout << "Total students above " << limit << ": " << cnt << endl;
    (void)it; (void)found;
}

int main() {
    StudentList students;
    const string FILE_NAME = "students.txt";
    int choice;

    do {
        cout << "\n===== Student Result Management System =====\n"
             << "1. Add student\n2. Display all\n3. Save to file\n"
             << "4. Load from file\n5. Students above a mark\n0. Exit\n"
             << "Enter choice: ";
        cin >> choice;
        switch (choice) {
            case 1: addStudent(students); break;
            case 2: displayAll(students); break;
            case 3: saveToFile(students, FILE_NAME); break;
            case 4: loadFromFile(students, FILE_NAME); break;
            case 5: showAbove(students); break;
            case 0: cout << "Goodbye!\n"; break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 0 && cin);
    return 0;
}