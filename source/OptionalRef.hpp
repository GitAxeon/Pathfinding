#pragma once

namespace asd
{
    
struct NullToken_t {};
inline constexpr NullToken_t NullOption {}; 

template<typename T>
class OptionalRef
{
public:
    OptionalRef() = default;
    ~OptionalRef() = default;

    OptionalRef(OptionalRef const&) = default;
    OptionalRef& operator=(OptionalRef const&) = default;

    OptionalRef(OptionalRef&&) = default;
    OptionalRef& operator=(OptionalRef&&) = default;

    OptionalRef(T&&) = delete;

    OptionalRef(NullToken_t) {};
    OptionalRef& operator=(NullToken_t) { m_Pointer = nullptr; return *this; }

    OptionalRef(T& value) : m_Pointer(&value) {}
    OptionalRef& operator=(T& value) { m_Pointer = &value; return *this; }
    
    constexpr operator bool() const { return static_cast<bool>(m_Pointer); }
    bool has_value() const { return static_cast<bool>(m_Pointer); }

    T* operator->()
    {
        assert(m_Pointer != nullptr);
        return m_Pointer;
    }
    
    const T* operator->() const
    {
        assert(m_Pointer != nullptr);
        return m_Pointer;
    }

    T& operator*()
    {
        assert(m_Pointer != nullptr);
        return *m_Pointer;
    }

    T const& operator*() const
    {
        assert(m_Pointer != nullptr);
        return *m_Pointer;       
    }

    T& value()
    { 
        assert(m_Pointer != nullptr);
        return *m_Pointer;
    }

    T const& value() const
    {
        assert(m_Pointer != nullptr);
        return *m_Pointer;
    }

private:
    T* m_Pointer{nullptr};
};

}