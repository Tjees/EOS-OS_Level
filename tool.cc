#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cctype>

void normalizeWord(std::string &word) {
    for (char &c : word) {
        c = std::tolower(c);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <word_to_count>" << std::endl;
        return 1;
    }

    std::string targetWord = argv[1];
    normalizeWord(targetWord);

    std::map<std::string, int> wordCount;

    std::string line;
    std::cout << "Voer tekst in (typ 'exit' om te stoppen):\n";
    while (true) {
        std::getline(std::cin, line);
        if (line == "exit") {
            break;
        }

        std::istringstream iss(line);
        std::string word;

        while (iss >> word) {
            normalizeWord(word);
            if (word == targetWord) {
                wordCount[word]++;
            }
        }
    }

    std::cout << "Het woord '" << targetWord << "' komt " << wordCount[targetWord] << " keer voor." << std::endl;

    return 0;
}