#ifndef HTTP_STRINGBUILDER_H
#define HTTP_STRINGBUILDER_H

#include <string>
#include <list>
#include <numeric>

// Subset of http://msdn.microsoft.com/en-us/library/system.text.stringbuilder.aspx
template <typename chr>
class StringBuilder {
    typedef std::basic_string<chr> string_t;
    typedef std::list<string_t> container_t; // Reasons not to use vector below.
    typedef typename string_t::size_type size_type; // Reuse the size type in the string.
    container_t m_Data;
    size_type m_totalSize;
    void append(string_t &&src) {
        m_Data.push_back(std::move(src));
        m_totalSize += src.size();
    }
    void append(const string_t &src) {
        m_Data.push_back(src);
        m_totalSize += src.size();
    }
    // No copy constructor, no assignement.
    StringBuilder(const StringBuilder &);
    StringBuilder & operator = (const StringBuilder &);
public:
    StringBuilder(const string_t &src) {
        if (!src.empty()) {
            m_Data.push_back(src);
        }
        m_totalSize = src.size();
    }
    StringBuilder() {
        m_totalSize = 0;
    }
    // TODO: Constructor that takes an array of strings.

    StringBuilder & Append(const string_t &src) {
        append(src);
        return *this; // allow chaining.
    }
    StringBuilder & Append(string_t &&src) {
        append(std::move(src));
        return *this; // allow chaining.
    }
    // This one lets you add any STL container to the string builder.
    template<class inputIterator>
    StringBuilder & Add(const inputIterator &first, const inputIterator &afterLast) {
        // std::for_each and a lambda look like overkill here.
        // <b>Not</b> using std::copy, since we want to update m_totalSize too.
        for (inputIterator f = first; f != afterLast; ++f) {
            append(*f);
        }
        return *this; // allow chaining.
    }
    StringBuilder & AppendLine(const string_t &src) {
        static chr lineFeed[] { 10, 0 }; // C++ 11. Feel the love!
        m_Data.push_back(src + lineFeed);
        m_totalSize += 1 + src.size();
        return *this; // allow chaining.
    }
    StringBuilder & AppendLine() {
        static chr lineFeed[] { 10, 0 };
        m_Data.push_back(lineFeed);
        ++m_totalSize;
        return *this; // allow chaining.
    }

    // TODO: AppendFormat implementation. Not relevant for the article.

    // Like C# StringBuilder.ToString()
    // Note the use of reserve() to avoid reallocations.
    string_t ToString() const {
        string_t result;
        // The whole point of the exercise!
        // If the container has a lot of strings, reallocation (each time the result grows) will take a serious toll,
        // both in performance and chances of failure.
        // I measured (in code I cannot publish) fractions of a second using 'reserve', and almost two minutes using +=.
        result.reserve(m_totalSize + 1);
        //	result = std::accumulate(m_Data.begin(), m_Data.end(), result); // This would lose the advantage of 'reserve'
        for (auto iter = m_Data.begin(); iter != m_Data.end(); ++iter) {
            result += *iter;
        }
        return result;
    }

    // like javascript Array.join()
    string_t Join(const string_t &delim) const {
        if (delim.empty()) {
            return ToString();
        }
        string_t result;
        if (m_Data.empty()) {
            return result;
        }
        // Hope we don't overflow the size type.
        size_type st = (delim.size() * (m_Data.size() - 1)) + m_totalSize + 1;
        result.reserve(st);
        // If you need reasons to love C++11, here is one.
        struct adder {
            string_t m_Joiner;
            adder(const string_t &s): m_Joiner(s) {
                // This constructor is NOT empty.
            }
            // This functor runs under accumulate() without reallocations, if 'l' has reserved enough memory.
            string_t operator()(string_t &l, const string_t &r) {
                l += m_Joiner;
                l += r;
                return l;
            }
        } adr(delim);
        auto iter = m_Data.begin();
        // Skip the delimiter before the first element in the container.
        result += *iter;
        return std::accumulate(++iter, m_Data.end(), result, adr);
    }

}; // class StringBuilder

#endif
