#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <iomanip>
#include <cctype>
#include <cstring>
#include <algorithm>

using namespace std;

const int MAX_ARTICLES = 44850;
const int MAX_FIELDS = 4;
const int MAX_WORDS = 1000;

// Structure to store each news article
struct Article {
    string title;
    string text;
    string subject;
    string date;
    int year, month;
    bool isFake;

    // Function to extract the year from the date format "DD-MM-YY"
    int getYear() const {
        if (date.size() >= 9) {
            int year = stoi(date.substr(7, 2));
            return (year < 50) ? (2000 + year) : (1900 + year); // Assume 2000+ for "00-49", else 1900+
        }
        return 0; // Invalid case
    }

    // Function to convert month name to numeric value (for sorting if needed)
    int getMonth() const {
        static map<string, int> monthMap = {
            {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4}, {"May", 5}, {"Jun", 6},
            {"Jul", 7}, {"Aug", 8}, {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12}
        };
    
        if (date.size() >= 9) {
            string month = date.substr(3, 3); // Extract "Jul"
            if (monthMap.count(month)) {
                return monthMap[month];
            }
        }
        return 0; // Default if invalid month
    }

    // Extracts the day
    int getDay() const {
        if (date.size() >= 9) {
            return stoi(date.substr(0, 2)); // Extract "22" from "22-Jul-16"
        }
        return 0;
    }
};

// Node structure for linked list
struct Node {
    Article article;
    Node* next;
};

// Function to properly split CSV lines while handling quotes
void parseCSVLine(const string &line, string fields[MAX_FIELDS]) {
    stringstream ss(line);
    string cell;
    bool insideQuote = false;
    string value;
    int fieldIndex = 0;

    for (char c : line) {
        if (c == '"') {
            insideQuote = !insideQuote; // Toggle quote status
        } else if (c == ',' && !insideQuote) {
            if (fieldIndex < MAX_FIELDS) {
                fields[fieldIndex++] = value;
                value = "";
            }
        } else {
            value += c;
        }
    }

    // Store last field
    if (fieldIndex < MAX_FIELDS) {
        fields[fieldIndex] = value;
    }
}

// NewsManager class using linked list
class NewsManager {
private:
    Node* head;
    int count;
    int countTrue;
    int countFake;
    bool isSorted;

public:
    NewsManager() : head(nullptr), count(0), countTrue(0), countFake(0), isSorted(false) {}

    ~NewsManager() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void loadArticlesFromCSV(const string &filename, bool isTrueNews) {
        ifstream file(filename);
        if (!file) {
            cout << "Error: Could not open file " << filename << endl;
            return;
        }
    
        string line;
        getline(file, line); // Skip header
    
        int fileCount = 0;  // Track how many articles were loaded from this file
    
        while (getline(file, line) && count < MAX_ARTICLES) {
            string fields[MAX_FIELDS];
            parseCSVLine(line, fields);
    
            if (!fields[0].empty() && !fields[1].empty() && !fields[2].empty() && !fields[3].empty()) {
                Node* newNode = new Node();
                newNode->article.title = fields[0];
                newNode->article.text = fields[1];
                newNode->article.subject = fields[2];
                newNode->article.date = fields[3];
                newNode->article.isFake = !isTrueNews;
                newNode->next = head;
                head = newNode;
                count++;   // Increment the global count
                fileCount++;  // Increment the file-specific count
            }
        }
    
        file.close();

        if (isTrueNews) {
            countTrue += fileCount;
        } else {
            countFake += fileCount;
        }

        cout << "Loaded " << fileCount << " articles from " << filename << endl;
        isSorted = false;  // Reset sorting flag after loading new articles
    }
    
    void displayArticleCounts() {
        cout << "\nTotal True News Articles: " << countTrue;
        cout << "\nTotal Fake News Articles: " << countFake;
        cout << "\nTotal Combined Articles: " << count << endl;
    }

    void displayArticles() {
        if (count == 0) {
            cout << "No articles available." << endl;
            return;
        }

        Node* current = head;
        while (current != nullptr) {
            cout << "\nTitle: " << current->article.title
                    << "\nCategory: " << current->article.subject
                    << "\nDate: " << current->article.date << "\n"
                    << "---------------------------------" << endl;
            current = current->next;
        }
    }

    int getCount() {
        return count;
    }

    // Merge function to combine two sorted halves
    Node* merge(Node* left, Node* right) {
        Node* result = nullptr;

        if (left == nullptr)
            return right;
        if (right == nullptr)
            return left;

        if (left->article.getYear() <= right->article.getYear()) {
            result = left;
            result->next = merge(left->next, right);
        } else {
            result = right;
            result->next = merge(left, right->next);
        }

        return result;
    }

    // Merge Sort function
    void mergeSort(Node** headRef) {
        Node* head = *headRef;
        Node* a;
        Node* b;

        if (head == nullptr || head->next == nullptr) {
            return;
        }

        split(head, &a, &b);

        mergeSort(&a);
        mergeSort(&b);

        *headRef = merge(a, b);
    }

    // Function to split the linked list into two halves
    void split(Node* source, Node** frontRef, Node** backRef) {
        Node* fast;
        Node* slow;
        slow = source;
        fast = source->next;

        while (fast != nullptr) {
            fast = fast->next;
            if (fast != nullptr) {
                slow = slow->next;
                fast = fast->next;
            }
        }

        *frontRef = source;
        *backRef = slow->next;
        slow->next = nullptr;
    }

    // Wrapper function to call mergeSort
    void sortArticlesByYear() {
        if (isSorted) {
            cout << "Articles are already sorted.\n";
            return;
        }

        mergeSort(&head);
        isSorted = true; // #Sorted
        cout << "\nArticles sorted by year month and day in ascending order.\n";
    }

    Node* binarySearchByYear(int year) {
        Node* current = head;
        Node* result = nullptr;

        while (current != nullptr) {
            if (current->article.getYear() == year) {
                result = current;
                break;
            }
            current = current->next;
        }

        return result; // Returns first node of the matching year or nullptr if not found
    }
    
    void searchArticlesByYear(int year) {
        Node* result = binarySearchByYear(year);
        if (result == nullptr) {
            cout << "No articles found for the year " << year << "\n";
            return;
        }
    
        cout << "\nArticles from year " << year << "\n";
        Node* current = result;
        while (current != nullptr && current->article.getYear() == year) {
            cout << "Title: " << current->article.title
                 << "\nCategory: " << current->article.subject
                 << "\nDate: " << current->article.date
                 << "\n--------------------------\n";
            current = current->next;
        }
    }
    
    void searchArticlesByCategory(const string &category) {
        bool found = false;
        cout << "\nArticles under category: " << category << "\n";
    
        Node* current = head;
        while (current != nullptr) {
            if (current->article.subject == category) {
                cout << "Title: " << current->article.title
                     << "\nCategory: " << current->article.subject
                     << "\nDate: " << current->article.date
                     << "\n--------------------------\n";
                found = true;
            }
            current = current->next;
        }
    
        if (!found) {
            cout << "No articles found in category: " << category << "\n";
        }
    }
    
    string monthToString(int month) {
        static const string months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
        return (month >= 1 && month <= 12) ? months[month - 1] : "Unknown";
    }

    void fakePoliticsPercentage() {
        map<int, int> totalArticlesPerMonth;
        map<int, int> fakeArticlesPerMonth;
    
        Node* current = head;
        while (current != nullptr) {
            int year = current->article.getYear();
            int month = current->article.getMonth();
    
            // Only consider political articles from 2016
            if (year == 2016 && (current->article.subject == "politics" || current->article.subject == "politicsNews")) {
                totalArticlesPerMonth[month]++;
                if (current->article.isFake) {
                    fakeArticlesPerMonth[month]++;
                }
            }
            current = current->next;
        }
    
        // Print results
        cout << "\nPercentage of Fake Political Articles in 2016\n\n";
        string monthNames[] = {"", "January", "February", "March", "April", "May", "June",
                               "July", "August", "September", "October", "November", "December"};
    
        for (int month = 1; month <= 12; month++) {
            int total = totalArticlesPerMonth[month];
            int fake = fakeArticlesPerMonth[month];
            int percentage = (total > 0) ? (fake * 100 / total) : 0;
    
            cout << left << setw(12) << monthNames[month] << " | ";
            cout << string(percentage, '*') << " " << percentage << "%" << endl;
        }
    }
    
    bool getIsSorted() const {
        return isSorted;
    }

    void mostFrequentWords() {
        // Arrays for storing word frequency
        string words[MAX_WORDS];
        int freq[MAX_WORDS] = {0};
        int wordCount = 0;

        Node* current = head;
        while (current != nullptr) {
            if (current->article.isFake && current->article.subject == "Government News") {
                string text = current->article.text;
                stringstream ss(text);
                string word;

                while (ss >> word) {
                    // Convert word to lowercase and remove non-alphabetic characters
                    transform(word.begin(), word.end(), word.begin(), ::tolower);
                    word.erase(remove_if(word.begin(), word.end(), [](char c) { return !isalpha(c); }), word.end());

                    if (!word.empty()) {
                        bool found = false;
                        // Check if the word already exists in the array
                        for (int j = 0; j < wordCount; j++) {
                            if (words[j] == word) {
                                freq[j]++;
                                found = true;
                                break;
                            }
                        }

                        // If the word is not found, add it
                        if (!found && wordCount < MAX_WORDS) {
                            words[wordCount] = word;
                            freq[wordCount] = 1;
                            wordCount++;
                        }
                    }
                }
            }
            current = current->next;
        }

        // Sort the words by frequency
        for (int i = 0; i < wordCount - 1; i++) {
            for (int j = i + 1; j < wordCount; j++) {
                if (freq[i] < freq[j]) {
                    swap(freq[i], freq[j]);
                    swap(words[i], words[j]);
                }
            }
        }

        // Display the most frequent words
        cout << "\nMost frequent words in fake Government news articles:\n";
        for (int i = 0; i < wordCount; i++) {
            cout << left << setw(15) << words[i] << " : " << freq[i] << endl;
        }
    }
};

// Main function
int main() {
    NewsManager news;
    
    // Load both datasets
    news.loadArticlesFromCSV("true.csv", true);
    news.loadArticlesFromCSV("fake_cleaned.csv", false);

    int choice;
    do {
        cout << "\nNews Articles Management System";
        cout << "\n1. Display articles";
        cout << "\n2. Sort articles by year";
        cout << "\n3. Search articles by year";
        cout << "\n4. Search articls by category";
        cout << "\n5. View percentage of fake political news articles in 2016";
        cout << "\n6. Most frequent government related fake news articles";
        cout << "\n7. Exit";
        cout << "\nEnter choice: ";
        cin >> choice;
        cin.ignore(); // Clear input buffer

        switch (choice) {
            case 1:
                news.displayArticles();
                break;
            case 2: 
                news.sortArticlesByYear();
                cout << "Articles have been sorted by year.\n";
                news.displayArticleCounts();
                break;
            case 3:
                int year;

                if(!news.getIsSorted()) {
                    cout << "\nRequired to sort articles first.";
                    cout << "\n------------------------------------"<< endl;
                    break;
                }
                cout << "Enter the year to search: ";
                cin >> year;
                cin.ignore(); // Clear input buffer
                news.searchArticlesByYear(year);
                break;
            case 4: {
                string category;
                cout << "Enter the category to search: ";
                getline(cin, category);
                news.searchArticlesByCategory(category);
                break;
            }
            case 5: 
                news.fakePoliticsPercentage();
                break;
            case 6: 
                news.mostFrequentWords();
                break;
            case 7:
                cout << "Exiting program..." << endl;
                break;
            default:
                cout << "Invalid choice, try again!" << endl;
        }
    } while (choice != 7);

    return 0;
}
