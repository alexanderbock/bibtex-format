/*****************************************************************************************
 *                                                                                       *
 * BSD 3-Clause License                                                                  *
 *                                                                                       *
 * Copyright (c) 2018, Alexander Bock                                                    *
 * All rights reserved.                                                                  *
 *                                                                                       *
 * Redistribution and use in source and binary forms, with or without modification, are  *
 * permitted provided that the following conditions are met:                             *
 *                                                                                       *
 * Redistributions of source code must retain the above copyright notice, this list of   *
 * conditions and the following disclaimer.                                              *
 *                                                                                       *
 * Redistributions in binary form must reproduce the above copyright noticem this list   *
 * of conditions and the following disclaimer in the documentation and/or other          *
 * materials provided with the distribution.                                             *
 *                                                                                       *
 * Neither the name of the copyright holder nor the names of its contributors may be     *
 * used to endorse or promote products derived from this software without specific prior *
 * written permission.                                                                   *
 *                                                                                       *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY   *
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT   *
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,        *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  *
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR    *
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN      *
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    *
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
 * DAMAGE.                                                                               *
 *                                                                                       *
*****************************************************************************************/

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

// Used in the operator[] to dump keywords without a member variable
std::string DummyString;

enum class Type {
    Unknown = -1,
    Article = 0,
    Book,
    Booklet,
    InBook,
    InCollection,
    InProceedings,
    Manual,
    MastersThesis,
    Misc,
    PhDThesis,
    Proceedings,
    TechReport,
    Unpublished
};

Type typeFromString(const std::string& type) {
    if (type == "article") {        return Type::Article;       }
    if (type == "book") {           return Type::Book;          }
    if (type == "booklet") {        return Type::Booklet;       }
    if (type == "conference") {     return Type::InProceedings; }
    if (type == "inbook") {         return Type::InBook;        }
    if (type == "incollection") {   return Type::InCollection;  }
    if (type == "inproceedings") {  return Type::InProceedings; }
    if (type == "manual") {         return Type::Manual;        }
    if (type == "mastersthesis") {  return Type::MastersThesis; }
    if (type == "misc") {           return Type::Misc;          }
    if (type == "phdthesis") {      return Type::PhDThesis;     }
    if (type == "proceedings") {    return Type::Proceedings;   }
    if (type == "techreport") {     return Type::TechReport;    }
    if (type == "unpublished") {    return Type::Unpublished;   }
    return Type::Unknown;
}

enum class Keyword {
    Unknown = -1,
    Address = 0,
    Author,
    BookTitle,
    Chapter,
    Doi,
    Edition,
    Editor,
    Institution,
    Journal,
    HowPublished,
    Key,
    Month,
    Note,
    Number,
    Organization,
    Pages,
    Publisher,
    School,
    Series,
    Title,
    Type,
    Url,
    Volume,
    Year
};

Keyword keywordFromString(const std::string& keyword) {
    if (keyword == "address") {         return Keyword::Address;        }
    if (keyword == "author") {          return Keyword::Author;         }
    if (keyword == "booktitle") {       return Keyword::BookTitle;      }
    if (keyword == "chapter") {         return Keyword::Chapter;        }
    if (keyword == "doi") {             return Keyword::Doi;            }
    if (keyword == "edition") {         return Keyword::Edition;        }
    if (keyword == "editor") {          return Keyword::Editor;         }
    if (keyword == "institution") {     return Keyword::Institution;    }
    if (keyword == "journal") {         return Keyword::Journal;        }
    if (keyword == "howpublished") {    return Keyword::HowPublished;   }
    if (keyword == "key") {             return Keyword::Key;            }
    if (keyword == "month") {           return Keyword::Month;          }
    if (keyword == "note") {            return Keyword::Note;           }
    if (keyword == "number") {          return Keyword::Number;         }
    if (keyword == "organization") {    return Keyword::Organization;   }
    if (keyword == "pages") {           return Keyword::Pages;          }
    if (keyword == "publisher") {       return Keyword::Publisher;      }
    if (keyword == "school") {          return Keyword::School;         }
    if (keyword == "series") {          return Keyword::Series;         }
    if (keyword == "title") {           return Keyword::Title;          }
    if (keyword == "type") {            return Keyword::Type;           }
    if (keyword == "url") {             return Keyword::Url;            }
    if (keyword == "volume") {          return Keyword::Volume;         }
    if (keyword == "year") {            return Keyword::Year;           }
    return Keyword::Unknown;
}

const std::map<Type, std::vector<Keyword>> AcceptedKeywords = {
    {
        Type::Article,
        {   
            Keyword::Author, Keyword::Title,  Keyword::Journal, Keyword::Year, 
            Keyword::Volume, Keyword::Number, Keyword::Pages,   Keyword::Month,
            Keyword::Note,   Keyword::Key,    Keyword::Doi
        }
    },
    {
        Type::Book,
        {
            Keyword::Title,   Keyword::Publisher, Keyword::Year,   Keyword::Author,
            Keyword::Editor,  Keyword::Volume,    Keyword::Number, Keyword::Series,
            Keyword::Address, Keyword::Edition,   Keyword::Month,  Keyword::Note,
            Keyword::Key,     Keyword::Url,       Keyword::Doi
        }
    },
    {
        Type::Booklet,
        {
            Keyword::Title, Keyword::Author, Keyword::HowPublished, Keyword::Address, 
            Keyword::Month, Keyword::Year,   Keyword::Note,         Keyword::Key,
            Keyword::Doi
        }
    },
    {
        Type::InBook,
        {
            Keyword::Title,     Keyword::Publisher, Keyword::Year,  Keyword::Author,
            Keyword::Editor,    Keyword::Chapter,   Keyword::Pages, Keyword::Volume,   
            Keyword::Number,    Keyword::Series,    Keyword::Type,  Keyword::Address,
            Keyword::Edition,   Keyword::Month,     Keyword::Note,  Keyword::Key,
            Keyword::Doi
        }
    },
    {
        Type::InCollection,
        {
            Keyword::Author,  Keyword::Title,   Keyword::BookTitle, Keyword::Publisher,
            Keyword::Year,    Keyword::Editor,  Keyword::Volume,    Keyword::Number,   
            Keyword::Series,  Keyword::Type,    Keyword::Chapter,   Keyword::Pages,
            Keyword::Address, Keyword::Edition, Keyword::Month,     Keyword::Note,
            Keyword::Key,     Keyword::Doi
        }
    },
    {
        Type::InProceedings,
        {
            Keyword::Author,    Keyword::Title,   Keyword::BookTitle, Keyword::Year,
            Keyword::Editor,    Keyword::Volume,  Keyword::Number,    Keyword::Series,
            Keyword::Pages,     Keyword::Address, Keyword::Month,   Keyword::Organization,
            Keyword::Publisher, Keyword::Note,    Keyword::Key,       Keyword::Doi
        }
    },
    {
        Type::Manual,
        {
            Keyword::Title,   Keyword::Author, Keyword::Organization, Keyword::Address,
            Keyword::Edition, Keyword::Month,  Keyword::Year,         Keyword::Note,
            Keyword::Key,     Keyword::Doi
        }
    },
    {
        Type::MastersThesis,
        {
            Keyword::Author, Keyword::Title,   Keyword::School, Keyword::Year,
            Keyword::Type,   Keyword::Address, Keyword::Month,  Keyword::Note,
            Keyword::Key,    Keyword::Doi
        }
    },
    {
        Type::Misc,
        {
            Keyword::Author, Keyword::Title, Keyword::HowPublished, Keyword::Month,
            Keyword::Year,   Keyword::Note,  Keyword::Key,          Keyword::Doi
        }
    },
    {
        Type::PhDThesis,
        {
            Keyword::Author, Keyword::Title, Keyword::School, Keyword::Year,
            Keyword::Type,   Keyword::Address, Keyword::Month,  Keyword::Note,
            Keyword::Key,    Keyword::Doi
        }
    },
    {
        Type::Proceedings,
        {
            Keyword::Title,    Keyword::Year,          Keyword::Editor,  Keyword::Volume,
            Keyword::Number,    Keyword::Series,       Keyword::Address, Keyword::Month,
            Keyword::Publisher, Keyword::Organization, Keyword::Note,    Keyword::Key,
            Keyword::Doi
        }
    },
    {
        Type::TechReport,
        {
            Keyword::Author, Keyword::Title,  Keyword::Institution, Keyword::Year,
            Keyword::Type,   Keyword::Number, Keyword::Address,     Keyword::Month,
            Keyword::Note,   Keyword::Key,    Keyword::Doi
        }
    },
    {
        Type::Unpublished,
        {
            Keyword::Author, Keyword::Title, Keyword::Note, Keyword::Month,
            Keyword::Year,   Keyword::Key,   Keyword::Doi
        }
    }
};

struct Entry {
    std::string& operator[](Keyword keyword) {
        switch (keyword) {
            case Keyword::Address:      return address;
            case Keyword::Author:       return author;
            case Keyword::BookTitle:    return bookTitle;
            case Keyword::Chapter:      return chapter;
            case Keyword::Doi:          return doi;
            case Keyword::Edition:      return edition;
            case Keyword::Editor:       return editor;
            case Keyword::Institution:  return institution;
            case Keyword::Journal:      return journal;
            case Keyword::HowPublished: return howPublished;
            case Keyword::Key:          return key;
            case Keyword::Month:        return month;
            case Keyword::Note:         return note;
            case Keyword::Number:       return number;
            case Keyword::Organization: return organization;
            case Keyword::Pages:        return pages;
            case Keyword::Publisher:    return publisher;
            case Keyword::School:       return school;
            case Keyword::Series:       return series;
            case Keyword::Title:        return title;
            case Keyword::Type:         return type;
            case Keyword::Url:          return url;
            case Keyword::Volume:       return volume;
            case Keyword::Year:         return year;
            default:                    return DummyString;
        }
    }

    const std::string& operator[](Keyword keyword) const {
        switch (keyword) {
            case Keyword::Address:      return address;
            case Keyword::Author:       return author;
            case Keyword::BookTitle:    return bookTitle;
            case Keyword::Chapter:      return chapter;
            case Keyword::Doi:          return doi;
            case Keyword::Edition:      return edition;
            case Keyword::Editor:       return editor;
            case Keyword::Institution:  return institution;
            case Keyword::Journal:      return journal;
            case Keyword::HowPublished: return howPublished;
            case Keyword::Key:          return key;
            case Keyword::Month:        return month;
            case Keyword::Note:         return note;
            case Keyword::Number:       return number;
            case Keyword::Organization: return organization;
            case Keyword::Pages:        return pages;
            case Keyword::Publisher:    return publisher;
            case Keyword::School:       return school;
            case Keyword::Series:       return series;
            case Keyword::Title:        return title;
            case Keyword::Type:         return type;
            case Keyword::Url:          return url;
            case Keyword::Volume:       return volume;
            case Keyword::Year:         return year;
            default:                    return DummyString;
        }
    }

    std::string citeKey;
    Type entryType;

    std::string address;
    std::string author;
    std::string bookTitle;
    std::string chapter;
    std::string doi;
    std::string edition;
    std::string editor;
    std::string institution;
    std::string journal;
    std::string howPublished;
    std::string key;
    std::string month;
    std::string note;
    std::string number;
    std::string organization;
    std::string pages;
    std::string publisher;
    std::string school;
    std::string series;
    std::string title;
    std::string type;
    std::string url;
    std::string volume;
    std::string year;
};

std::vector<std::string> checkCompleteness(const Entry& entry) {
    auto checkRequired = [](const Entry& entry,
                            std::vector<Keyword> keywords,
                            std::vector<std::pair<Keyword, Keyword>> keywordPairs = {})
        -> std::vector<std::string>
    {
        auto keywordToString = [](Keyword keyword) -> std::string {
            switch (keyword) {
                case Keyword::Address:      return "address";
                case Keyword::Author:       return "author";
                case Keyword::BookTitle:    return "bookTitle";
                case Keyword::Chapter:      return "chapter";
                case Keyword::Doi:          return "doi";
                case Keyword::Edition:      return "edition";
                case Keyword::Editor:       return "editor";
                case Keyword::Institution:  return "institution";
                case Keyword::Journal:      return "journal";
                case Keyword::HowPublished: return "howPublished";
                case Keyword::Key:          return "key";
                case Keyword::Month:        return "month";
                case Keyword::Note:         return "note";
                case Keyword::Number:       return "number";
                case Keyword::Organization: return "organization";
                case Keyword::Pages:        return "pages";
                case Keyword::Publisher:    return "publisher";
                case Keyword::School:       return "school";
                case Keyword::Series:       return "series";
                case Keyword::Title:        return "title";
                case Keyword::Type:         return "type";
                case Keyword::Url:          return "url";
                case Keyword::Volume:       return "volume";
                case Keyword::Year:         return "year";
                default:                    return "";
            }
        };
        
        bool failed = std::any_of(
            keywords.begin(),
            keywords.end(),
            [entry](Keyword k) { return entry[k].empty(); }
        );

        if (!keywordPairs.empty()) {
            failed &= std::any_of(
                keywordPairs.begin(),
                keywordPairs.end(),
                [entry](const std::pair<Keyword, Keyword>& p) {
                return entry[p.first].empty() || entry[p.second].empty();
            }
            );
        }

        if (failed) {
            std::vector<std::string> result;
            std::for_each(
                keywords.begin(),
                keywords.end(),
                [&result, entry, keywordToString](Keyword k) { 
                    if (entry[k].empty()) {
                        result.push_back(keywordToString(k));
                    }
                }
            );

            std::for_each(
                keywordPairs.begin(),
                keywordPairs.end(),
                [&result, entry, keywordToString](const std::pair<Keyword, Keyword>& p) {
                    if (entry[p.first].empty() || entry[p.second].empty()) {
                        result.push_back(
                            keywordToString(p.first) + " or " + keywordToString(p.second)
                        );
                    }
                }
            );
            return result;
        }
        else {
            return {};
        }
    };
    switch (entry.entryType) {
        case Type::Article:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::Journal, Keyword::Year,
                  Keyword::Volume }
            );
        case Type::Book:
            return checkRequired(
                entry,
                { Keyword::Title, Keyword::Publisher, Keyword::Year },
                { { Keyword::Author, Keyword::Editor } }
            );
        case Type::Booklet:
            return checkRequired(entry, { Keyword::Title });
        case Type::InBook:
            return checkRequired(
                entry,
                { Keyword::Title, Keyword::Publisher, Keyword::Year },
                { { Keyword::Author, Keyword::Editor },
                  { Keyword::Chapter, Keyword::Pages } }
            );
        case Type::InCollection:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::BookTitle, Keyword::Publisher,
                  Keyword::Year }
            );
        case Type::InProceedings:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::BookTitle, Keyword::Year }
            );
        case Type::Manual:
            return checkRequired(entry, { Keyword::Title });
        case Type::MastersThesis:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::School, Keyword::Year }
            ); 
        case Type::Misc:
            return {};
        case Type::PhDThesis:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::School, Keyword::Year }
            );
        case Type::Proceedings:
            return checkRequired(
                entry,
                { Keyword::Title, Keyword::Year }
            );
        case Type::TechReport:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::Institution, Keyword::Year }
            );
        case Type::Unpublished:
            return checkRequired(
                entry,
                { Keyword::Author, Keyword::Title, Keyword::Note}
            );
        default:
            return {};
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Missing argument for BibTex file\n";
        return -1;
    }

    std::string contents(
        (std::istreambuf_iterator<char>(std::ifstream(argv[1]))),
        std::istreambuf_iterator<char>()
    );

    // Match:
    // @.... {
    //  .....\n
    //  .....\n
    //  .....
    // }
    std::regex regex(
        "@(.*)\\{((.*)(\\n)*)*?\\}",
        std::regex_constants::ECMAScript | std::regex_constants::icase
    );

    std::vector<Entry> entries;
    std::smatch match;
    while (std::regex_search(contents, match, regex)) {
        std::string reference = match.str();

        Entry entry;
        std::vector<std::string> extraFields;

        size_t firstBracket = reference.find_first_of('{');

        std::string typeInfo = reference.substr(1, firstBracket - 1);
        entry.entryType = typeFromString(typeInfo);

        size_t firstComma = reference.find_first_of(',', firstBracket);
        entry.citeKey = reference.substr(firstBracket + 1, firstComma - firstBracket - 1);

        if (entry.entryType == Type::Unknown) {
            std::cerr << "Error in: " << entry.citeKey << '\n';
            std::cerr << "Unknown type: " << typeInfo << "\n";

        }

        reference = reference.substr(firstComma + 1 + 1); // ,\n
        while (!reference.empty()) {
            // Removes whitespaces from the front and the end of the string
            auto trim = [](std::string& string) {
                string.erase(
                    string.begin(),
                    std::find_if(
                        string.begin(),
                        string.end(),
                        [](int ch) { return !std::isspace(ch); }
                    )
                );
                string.erase(
                    std::find_if(
                        string.rbegin(),
                        string.rend(),
                        [](int ch) { return !std::isspace(ch); }
                    ).base(),
                    string.end()
                );
            };

            trim(reference);

            size_t endLine = reference.find_first_of('\n');

            std::string line = reference.substr(0, endLine);

            if (line.back() == ',') {
                line = line.substr(0, line.size() - 1);
            }

            // There might not need to be a space between the keyword and the =
            size_t endKeywordSpace = line.find_first_of(' ');
            size_t endKeywordEquals = line.find_first_of('=');
            size_t endKeyword = std::min(endKeywordSpace, endKeywordEquals);
            std::string keyword = line.substr(0, endKeyword);

            std::string value = line.substr(endKeywordEquals + 1);

            trim(value);

            // Remove the "" or {} pair around the value
            value = value.substr(1, value.size() - 2);

            Keyword kw = keywordFromString(keyword);
            // No need to check for keywords as they will be added to the 'extraFields'
            // vector either way
            
            std::vector<Keyword> accepted = AcceptedKeywords.at(entry.entryType);
            bool keywordAllowed = std::find(
                accepted.begin(),
                accepted.end(),
                kw
            ) != accepted.end();

            if (keywordAllowed) {
                entry[keywordFromString(keyword)] = value;
            }
            else {
                extraFields.push_back(keyword);
            }


            if (endLine == std::string::npos) {
                break;
            }
            reference = reference.substr(endLine + 1 + 1); // ,\n  or \n}
        }


        std::vector<std::string> errors = checkCompleteness(entry);

        if (!extraFields.empty() || !errors.empty()) {
            std::cerr << "Error in: " << entry.citeKey << '\n';

            for (const std::string& missing : errors) {
                std::cerr << "Missing: " << missing << '\n';
            }

            for (const std::string& extra : extraFields) {
                std::cerr << "Extra:   " << extra << '\n';
            }

            std::cerr << '\n' << '\n';
        }

        entries.push_back(std::move(entry));

        contents = match.suffix();
    }

    return 0;
}
