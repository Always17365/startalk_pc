//
// Created by may on 2018/8/15.
//

#ifndef LIBEVENTTEST_PARSEFAILEDEXCEPTION_H
#define LIBEVENTTEST_PARSEFAILEDEXCEPTION_H

#include <exception>
#include <string>

namespace QTalk{
    namespace Excetpion {
        class ParseFailedException : public std::exception {

        public:
            ParseFailedException(const char *what) {
                this->errMessage = what;
            }

            const char *what() const throw() {
                return "C++ Exception";
            }

        private:
            std::string errMessage;

        };
    }
}

#endif //LIBEVENTTEST_PARSEFAILEDEXCEPTION_H
