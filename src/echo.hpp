#ifndef QUERY_ECHO_HPP
#define QUERY_ECHO_HPP

class stdout_wrapper
{
public:
    stdout_wrapper() {}
    stdout_wrapper(stdout_wrapper&& other)
      : stream_(other.stream_.str()) {}
    ~stdout_wrapper() { std::cout << stream_.str() << std::endl; }

    template <typename T>
    stdout_wrapper& operator<<(T const& value) 
    {
        stream_ << value;
        return *this;
    }

private:
    std::ostringstream stream_;
};

stdout_wrapper echo()
{
    return stdout_wrapper();
}

#endif

