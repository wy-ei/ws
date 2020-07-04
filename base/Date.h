//
// Created by dodo on 2020/7/4.
//

#ifndef WS_DATE_H
#define WS_DATE_H

#include <string>
#include <ctime>

class Date {
    friend std::ostream& operator<<(std::ostream& os, const Date& date){
        os << date.to_utc_string();
        return os;
    }
public:
    Date(): time_(time(nullptr)){}
    explicit Date(time_t time): time_(time){}

    std::string to_utc_string() const;
    std::string to_string(const std::string& fmt) const;
    std::string to_locale_string() const;
    std::string to_locale_string(const std::string& fmt) const;

    static
    Date from_utc_string(const std::string&);

    static
    Date from_locale_time_string(const std::string& fmt, const std::string&);


    bool operator<(const Date& rhs){
        return time_ < rhs.time_;
    }
    bool operator>(const Date& rhs){
        return time_ > rhs.time_;
    }
    bool operator==(const Date& rhs){
        return time_ == rhs.time_;
    }
    bool operator<=(const Date& rhs){
        return time_ <= rhs.time_;
    }
    bool operator>=(const Date& rhs){
        return time_ >= rhs.time_;
    }
private:
    time_t time_;
};


#endif //WS_DATE_H
