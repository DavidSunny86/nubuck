#pragma once

namespace GEN {

    class Uncopyable {
    private:
        Uncopyable(const Uncopyable& other);
        Uncopyable& operator=(const Uncopyable& other);
    public:
        Uncopyable(void) { }
        ~Uncopyable(void) { }
    };

} // namespace GEN
